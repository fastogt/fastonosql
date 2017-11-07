#!/usr/bin/env python
import json
import shlex
import sys

import build
import config
import pika
from pybuild_utils.base import system_info, utils


def gen_routing_key(platform, arch) -> str:
    return platform + '_' + arch


def print_usage():
    print("Usage:\n"
          "[optional] argv[1] platform\n"
          "[optional] argv[2] architecture\n")


class BuildRpcServer(object):
    EXCHANGE = 'build_servers_exchange'
    EXCHANGE_TYPE = 'direct'

    def __init__(self, platform, arch_bit):
        self.connection_ = None
        self.channel_ = None
        self.closing_ = False
        self.consumer_tag_ = None
        self.platform_ = platform
        self.arch_bit_ = arch_bit
        self.routing_key_ = gen_routing_key(platform, arch_bit)
        print("Build server for %s inited!" % platform)

    def connect(self):
        credentials = pika.PlainCredentials(config.USER_NAME, config.PASSWORD)
        params = pika.ConnectionParameters(host=config.REMOTE_HOST, credentials=credentials)
        return pika.SelectConnection(params, self.on_connection_open, stop_ioloop_on_close=False)

    def reconnect(self):
        self.connection_.ioloop.stop()

        if not self.closing_:
            self.connection_ = self.connect()
            self.connection_.ioloop.start()

    def on_connection_open(self, unused_connection):
        self.add_on_connection_close_callback()
        self.open_channel()

    def add_on_connection_close_callback(self):
        self.connection_.add_on_close_callback(self.on_connection_closed)

    def open_channel(self):
        self.connection_.channel(on_open_callback=self.on_channel_open)

    def on_channel_open(self, channel):
        self.channel_ = channel
        self.add_on_channel_close_callback()
        self.setup_exchange(self.EXCHANGE)

    def add_on_channel_close_callback(self):
        self.channel_.add_on_close_callback(self.on_channel_closed)

    def setup_exchange(self, exchange_name):
        print("setup_exchange: {0}".format(exchange_name))
        self.channel_.exchange_declare(self.on_exchange_declareok, exchange_name, self.EXCHANGE_TYPE)

    def on_exchange_declareok(self, unused_frame):
        self.setup_queue(self.routing_key_)

    def setup_queue(self, queue_name):
        print("queue_name: {0}".format(queue_name))
        self.channel_.queue_declare(self.on_queue_declareok, queue_name)

    def on_queue_declareok(self, method_frame):
        self.channel_.queue_bind(self.on_bindok, self.routing_key_, self.EXCHANGE, self.routing_key_)

    def on_bindok(self, unused_frame):
        self.start_consuming()

    def start_consuming(self):
        self.add_on_cancel_callback()
        print("Awaiting RPC build requests")
        self.consumer_tag_ = self.channel_.basic_consume(self.on_request, self.routing_key_)

    def on_consumer_cancelled(self, method_frame):
        if self.channel_:
            self.channel_.close()

    def add_on_cancel_callback(self):
        self.channel_.add_on_cancel_callback(self.on_consumer_cancelled)

    def on_channel_closed(self, channel, reply_code, reply_text):
        print("on_channel_closed reply_code: {0}, reply_text: {1}".format(reply_code, reply_text))
        self.connection_.close()

    def on_connection_closed(self, connection, reply_code, reply_text):
        print("on_connection_closed reply_code: {0}, reply_text: {1}".format(reply_code, reply_text))
        self.channel_ = None
        if self.closing_:
            self.connection_.ioloop.stop()
        else:
            self.connection_.add_timeout(5, self.reconnect)

    def run(self):
        self.connection_ = self.connect()
        self.connection_.ioloop.start()

    def build_package(self, platform, arch_bit, op_id, branding_options, package_types, destination, routing_key):
        build_request = build.BuildRequest(platform, arch_bit)
        platform = build_request.platform()
        arch = platform.arch()

        platform_and_arch_str = '{0}_{1}'.format(platform.name(), arch.name())
        dir_name = 'build_{0}_for_{1}'.format(platform_and_arch_str, op_id)
        bs = build.get_supported_build_system_by_name('ninja')

        self.send_status(routing_key, op_id, 20.0, 'Building package')

        def store(progress_min, progress_max, routing_key, op_id):
            def closure(progress, message):
                diff = progress_max - progress_min
                perc = round(progress_min + diff * (progress / 100.0), 1)
                print('{0}% {1}'.format(perc, message))
                sys.stdout.flush()
                self.send_status(routing_key, op_id, perc, message)

            return closure

        store = store(21.0, 79.0, routing_key, op_id)

        saver = build.ProgressSaver(store)
        file_paths = build_request.build('..', branding_options, dir_name, bs, package_types, saver)
        file_path = file_paths[0]
        self.send_status(routing_key, op_id, 80.0, 'Loading package to server')
        try:
            result = config.post_install_step(file_path, destination)
        except Exception as ex:
            raise ex

        return result

    def send_status(self, routing_key, op_id, progress, status):
        json_to_send = {'progress': progress, 'status': status}
        properties = pika.BasicProperties(content_type='application/json', correlation_id=op_id,
                                          headers={'type': 'status'})
        if self.channel_:
            self.channel_.basic_publish(exchange='',
                                        routing_key=routing_key,
                                        properties=properties,
                                        body=json.dumps(json_to_send))

    def send_response(self, routing_key, op_id, body):
        properties = pika.BasicProperties(
            content_type='application/json', correlation_id=op_id,
            headers={'type': 'response'})
        if self.channel_:
            self.channel_.basic_publish(exchange='',
                                        routing_key=routing_key,
                                        properties=properties,
                                        body=body)

    def on_request(self, ch, method, props, body):
        platform_and_arch = '{0}_{1}'.format(self.platform_, self.arch_bit_)
        if isinstance(body, bytes):
            body = body.decode("utf-8")

        data = json.loads(body)
        # self.acknowledge_message(method.delivery_tag)
        # return

        branding_variables = data.get('branding_variables')
        package_type = data.get('package_type')
        destination = data.get('destination')
        op_id = props.correlation_id
        package_types = [package_type]

        self.send_status(props.reply_to, op_id, 0.0, 'Prepare to build package')
        print('Build started for: {0}, platform: {1}'.format(op_id, platform_and_arch))
        try:
            response = self.build_package(self.platform_, self.arch_bit_, op_id, shlex.split(branding_variables),
                                          package_types, destination, props.reply_to)
            print('Build finished for: {0}, platform: {1}, response: {2}'.format(op_id, platform_and_arch, response))
            json_to_send = {'body': response}
        except utils.BuildError as ex:
            print('Build finished for: {0}, platform: {1}, exception: {2}'.format(op_id, platform_and_arch, str(ex)))
            json_to_send = {'error': str(ex)}
        except Exception as ex:
            print('Build finished for: {0}, platform: {1}, exception: {2}'.format(op_id, platform_and_arch, str(ex)))
            json_to_send = {'error': str(ex)}

        self.send_status(props.reply_to, op_id, 100.0, 'Completed')
        self.send_response(props.reply_to, op_id, json.dumps(json_to_send))
        self.acknowledge_message(method.delivery_tag)

    def acknowledge_message(self, delivery_tag):
        if self.channel_:
            self.channel_.basic_ack(delivery_tag)


if __name__ == "__main__":
    argc = len(sys.argv)

    if argc > 1:
        platform_str = sys.argv[1]
    else:
        platform_str = system_info.get_os()

    if argc > 2:
        arch_str = sys.argv[2]
    else:
        arch_str = system_info.get_arch_name()

    server = BuildRpcServer(platform_str, arch_str)
    server.run()
