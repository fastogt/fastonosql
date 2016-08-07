#!/usr/bin/env python
import pika
import json
import sys
import base
import config

def print_usage():
    print("Usage:\n"
        "[required] argv[1] operation id\n"
        "[optional] argv[2] platform\n"
        "[optional] argv[3] architecture\n"
        "[optional] argv[4] build args\n"
        "[optional] argv[5] package_type\n"
        "[optional] argv[6] destination\n")

class BuildRpcClient(object):
    def __init__(self):
        credentials = pika.PlainCredentials(config.USER_NAME, config.PASSWORD)
        self.connection = pika.BlockingConnection(pika.ConnectionParameters(host=config.SERVER_HOST, credentials = credentials))
        self.channel = self.connection.channel()

        result = self.channel.queue_declare(exclusive = True)
        self.callback_queue = result.method.queue
        self.channel.basic_consume(self.on_response, no_ack = True, queue = self.callback_queue)

    def build_request(self, op_id, platform, arch, branding_variables, package_type, destination):
        self.response = None
        self.corr_id = op_id
        request_data_json = {
            'branding_variables': branding_variables,
            'platform': platform,
            'arch': arch,
            'package_type' : package_type,
            'destination' : destination
        }
        request_data_str = json.dumps(request_data_json)
        print ("build request body: %s" % request_data_str)
        self.channel.basic_publish(exchange = '',
                           routing_key = base.gen_routing_key(platform, str(arch)),
                           properties = pika.BasicProperties(reply_to = self.callback_queue, correlation_id = self.corr_id),
                           body = request_data_str)
        self.response = None
        while self.response is None:
            self.connection.process_data_events()
        return self.response

    def on_response(self, ch, method, props, body):
        self.response = body

if __name__ == "__main__":

    argc = len(sys.argv)

    if argc > 1:
        op_id = sys.argv[1]
    else:
        print("Operation id not passed!")
        print_usage()
        quit()

    if argc > 2:
        platform_str = sys.argv[2]
    else:
        platform_str = base.get_os()

    platform_or_none = base.get_supported_platform_by_name(platform_str)
    if platform_or_none == None:
        print("Not supported platform: %s" % platform_str)
        print_usage()
        quit()

    if argc > 3:
        arch_str = sys.argv[3]
    else:
        arch_str = base.get_arch()

    if argc > 4:
        args = sys.argv[4]
    else:
        args = ''

    if argc > 5:
        package_type = sys.argv[5]
    else:
        package_type = platform_or_none.package_types[0]

    if argc > 6:
        destination = sys.argv[6]
    else:
        destination = '/tmp/'

    build_rpc = BuildRpcClient()
    response = build_rpc.build_request(op_id, platform_str, int(arch_str), args, destination)
    print("responce: %r" % response)
