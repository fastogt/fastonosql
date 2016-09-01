#!/usr/bin/env python
import pika
import json
import subprocess
import os
import shutil
import sys
import shlex
import re
from base import system_info
import config

class BuildError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return self.value

def print_usage():
    print("Usage:\n"
        "[optional] argv[1] platform\n"
        "[optional] argv[2] architecture\n")

def run_command(cmd):
    return subprocess.check_call(cmd)


class BuildRpcServer(object):
    def __init__(self, platform):
        credentials = pika.PlainCredentials(config.USER_NAME, config.PASSWORD)
        self.platform = platform
        self.connection = pika.BlockingConnection(pika.ConnectionParameters(host = config.REMOTE_HOST, credentials = credentials))
        self.channel = self.connection.channel()
        self.channel.queue_declare(queue = platform)
        self.channel.basic_qos(prefetch_count = 1)
        self.channel.basic_consume(self.on_request, queue = platform)
        print("Build server for %s created!\n" % platform)

    def start(self):
        print("Awaiting RPC build requests")
        self.channel.start_consuming()

    def build_package(self, op_id, platform, arch, branding_variables, package_type, destination, status_channel, routing_key):

        platform_or_none = system_info.get_supported_platform_by_name(platform)

        if platform_or_none == None:
            raise BuildError('invalid platform')

        if not arch in platform_or_none.archs:
            raise BuildError('invalid arch')
        if not package_type in platform_or_none.package_types:
            raise BuildError('invalid package_type')

        pwd = os.getcwd()
        dir_name = 'build_{0}_for_{1}'.format(platform, op_id)
        if os.path.exists(dir_name):
            shutil.rmtree(dir_name)

        os.mkdir(dir_name)
        os.chdir(dir_name)

        arch_args = '-DOS_ARCH={0}'.format(arch)
        generator_args = '-DCPACK_GENERATOR={0}'.format(package_type)
        log_to_file_args = '-DLOG_TO_FILE=ON'
        openssl_args = '-DOPENSSL_USE_STATIC=ON'
        branding_variables_list = shlex.split(branding_variables)
        cmake_line = ['cmake', '../../../', '-GNinja', '-DCMAKE_BUILD_TYPE=RELEASE', generator_args, arch_args, log_to_file_args, openssl_args]
        cmake_line.extend(branding_variables_list)
        try:
            run_command(cmake_line)
        except subprocess.CalledProcessError as ex:
            os.chdir(pwd)
            raise ex
            
        self.send_status(status_channel, routing_key, op_id, 20, 'Building package')
                         
        make_line = ['ninja', 'package']
        try:
            run_command(make_line)
        except subprocess.CalledProcessError as ex:
            os.chdir(pwd)
            raise ex

        self.send_status(status_channel, routing_key, op_id, 70, 'Stable package')
        
        in_file = open('CPackConfig.cmake', 'r')
        for line in in_file.readlines():
            res = re.search(r'SET\(CPACK_SOURCE_PACKAGE_FILE_NAME "(.+)"\)', line)
            if res != None:
                filename = res.group(1) + '.' + system_info.get_extension_by_package(package_type)
        in_file.close()

        try:
            self.send_status(status_channel, routing_key, op_id, 80, 'Loading package to server')
            result = config.post_install_step(filename, destination)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        os.chdir(pwd)
        return result

    def send_status(self, channel, routing_key, op_id, progress, status):
        json_to_send = {'progress': progress, 'status' : status}
        channel.basic_publish(exchange='', 
                         routing_key=routing_key,
                         properties = pika.BasicProperties(content_type = 'application/json', correlation_id = op_id, headers = {'type' : 'status'}),
                         body=json.dumps(json_to_send))
                         
    def on_request(self, ch, method, props, body):
        data = json.loads(body)

        branding_variables = data.get('branding_variables')
        platform = data.get('platform')
        arch = data.get('arch')
        package_type = data.get('package_type')
        destination = data.get('destination')
        op_id = props.correlation_id
        
        self.send_status(ch, props.reply_to, op_id, 0, 'Prepare to build package')
                              
        print('build started for: {0}, platform: {1}_{2}'.format(op_id, platform, arch))
        try:
            response = self.build_package(op_id, platform, arch, branding_variables, package_type, destination, ch, props.reply_to)
            print('build finished for: {0}, platform: {1}_{2}, responce: {3}'.format(op_id, platform, arch, response))
            json_to_send = {'body' :response}
        except Exception as ex:
            print('build finished for: {0}, platform: {1}_{2}, exception: {3}'.format(op_id, platform, arch, str(ex)))
            json_to_send = {'error': str(ex)}
        
        self.send_status(ch, props.reply_to, op_id, 100, 'Completed')

        ch.basic_publish(exchange = '',
                 routing_key = props.reply_to,
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
        
    server = BuildRpcServer(system_info.gen_routing_key(platform_str, arch_str))
    server.start()
