#!/usr/bin/env python
import pika
import json
import subprocess
import sys
import shlex
import socket
from base import system_info
import config
import build

def print_usage():
    print("Usage:\n"
        "[optional] argv[1] platform\n"
        "[optional] argv[2] architecture\n")

def run_command(cmd):
    return subprocess.check_call(cmd)

class BuildRpcServer(object):
    def __init__(self, platform, arch_bit):
        routing_key = system_info.gen_routing_key(platform, str(arch_bit))
        credentials = pika.PlainCredentials(config.USER_NAME, config.PASSWORD)
        params = pika.ConnectionParameters(host = config.REMOTE_HOST, credentials = credentials)
        self.connection = pika.BlockingConnection(params)
        self.channel = self.connection.channel()
        self.channel.queue_declare(queue = routing_key)
        self.channel.basic_qos(prefetch_count = 1)
        self.channel.basic_consume(self.on_request, queue = routing_key)
        self.buid = build.BuildRequest(platform, arch_bit)
        print("Build server for %s created!\n" % platform)

    def start(self):
        print("Awaiting RPC build requests")
        self.channel.start_consuming()

    def build_package(self, op_id, branding_options, package_types, destination, status_channel, routing_key):
        platform = self.buid.platform()
        arch = platform.arch()

        platform_and_arch_str = '{0}_{1}'.format(platform.name(), arch.name())
        dir_name = 'build_{0}_for_{1}'.format(platform_and_arch_str, op_id)

        self.send_status(status_channel, routing_key, op_id, 20, 'Building package')
        filenames = self.buid.build('..', branding_options, dir_name, package_types)
        filename = filenames[0]
        self.send_status(status_channel, routing_key, op_id, 80, 'Loading package to server')
        try:
            result = config.post_install_step(filename, destination)
        except Exception as ex:
            raise ex

        return result

    def send_status(self, channel, routing_key, op_id, progress, status):
        json_to_send = {'progress': progress, 'status' : status}
        try:
            channel.basic_publish(exchange='',
                         routing_key=routing_key,
                         properties = pika.BasicProperties(content_type = 'application/json', correlation_id = op_id, headers = {'type' : 'status'}),
                         body=json.dumps(json_to_send))
        except socket.error as ex:
            print('send_status, exception: {0}'.format(str(ex)))
                         
    def on_request(self, ch, method, props, body):
        data = json.loads(body)

        branding_variables = data.get('branding_variables')
        platform = data.get('platform')
        arch = data.get('arch')
        package_type = data.get('package_type')
        destination = data.get('destination')
        op_id = props.correlation_id
        package_types = []
        package_types.append(package_type)

        self.send_status(ch, props.reply_to, op_id, 0, 'Prepare to build package')
        print('Build started for: {0}, platform: {1}_{2}'.format(op_id, platform, arch))
        try:
            response = self.build_package(op_id, shlex.split(branding_variables), package_types, destination, ch, props.reply_to)
            print('Build finished for: {0}, platform: {1}_{2}, responce: {3}'.format(op_id, platform, arch, response))
            json_to_send = {'body' :response}
        except build.BuildError as ex:
            print('Build finished for: {0}, platform: {1}_{2}, exception: {3}'.format(op_id, platform, arch, str(ex)))
            json_to_send = {'error': str(ex)}
        except Exception as ex:
            print('Build finished for: {0}, platform: {1}_{2}, exception: {3}'.format(op_id, platform, arch, str(ex)))
            json_to_send = {'error': str(ex)}
        
        self.send_status(ch, props.reply_to, op_id, 100, 'Completed')

        ch.basic_publish(exchange = '', routing_key = props.reply_to,
                 properties = pika.BasicProperties(content_type = 'application/json', correlation_id = op_id, headers = {'type' : 'responce'} ),                 
                 body = json.dumps(json_to_send))
        ch.basic_ack(delivery_tag = method.delivery_tag)

if __name__ == "__main__":
    argc = len(sys.argv)

    if argc > 1:
        platform_str = sys.argv[1]
    else:
        platform_str = system_info.get_os()

    if argc > 2:
        arch_str = sys.argv[2]
    else:
        arch_str = system_info.get_arch_bit()
        
    server = BuildRpcServer(platform_str, int(arch_str))
    server.start()
