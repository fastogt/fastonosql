#!/usr/bin/env python
import subprocess
import sys
import os
import shutil
import re
from base import system_info

class BuildError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return self.value

def print_usage():
    print("Usage:\n"
        "[required] argv[1] cmake_root_path\n"
        "[optional] argv[2] branding_file_path\n"
        "[optional] argv[3] platform\n"
        "[optional] argv[4] architecture\n"
        "[optional] argv[5] packages for example(\"DEB RPM\")\n")

def run_command_cb(cmd, cb):
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    while True:
        output = process.stdout.readline()
        if output == '' and process.poll() is not None:
            break
        if output:
            line = output.strip()
            if cb:
                cb(line)
    rc = process.poll()
    return rc

def read_file_line_by_line(file):
    if not os.path.exists(file):
        raise BuildError('file path: %s not exists' % file)

    file_array = []
    with open(file, "r") as ins:
        for line in ins:
            file_array.append(line.strip('\n'))

    return file_array

class ProgressSaverBase(object):
    def __init__(self):
        self.progress_ = 0.0

    def process(self, line):
        #print line
        return

    def update_progress(self, progress):
        self.progress_ = progress

class BuildRequest(object):
    def __init__(self, platform, arch_bit):
        platform_or_none = system_info.get_supported_platform_by_name(platform)

        if platform_or_none == None:
            raise BuildError('invalid platform')

        arch = platform_or_none.architecture_by_bit(arch_bit)
        if arch == None:
            raise BuildError('invalid arch')

        self.platform_ = system_info.Platform(platform_or_none.name(), arch, platform_or_none.package_types())
        print("Build request for platform: {0}, arch: {1} created".format(platform, arch.name()))

    def platform(self):
        return self.platform_

    def build(self, cmake_project_root_path, branding_options, dir_path, package_types, saver):
        cmake_project_root_abs_path = os.path.abspath(cmake_project_root_path)
        if not os.path.exists(cmake_project_root_abs_path):
            raise BuildError('invalid cmake_project_root_path: %s' % cmake_project_root_path)

        if not package_types:
            package_types = self.platform_.package_types()

        abs_dir_path = os.path.abspath(dir_path)
        if os.path.exists(abs_dir_path):
            shutil.rmtree(abs_dir_path)

        saver.process("Start building project "
              "branding_options:\n{0}".format("\n".join(branding_options)))

        pwd = os.getcwd()
        os.mkdir(abs_dir_path)
        os.chdir(abs_dir_path)

        # project static options
        arch = self.platform_.arch()
        arch_args = '-DOS_ARCH={0}'.format(arch.bit)
        log_to_file_args = '-DLOG_TO_FILE=ON'
        openssl_args = '-DOPENSSL_USE_STATIC=ON'
        cmake_line = ['cmake', cmake_project_root_abs_path, '-GNinja', '-DCMAKE_BUILD_TYPE=RELEASE', arch_args, log_to_file_args, openssl_args]

        if branding_options:
            cmake_line.extend(branding_options)

        saver.update_progress(10.0)
        try:
            run_command_cb(cmake_line, saver.process)
        except subprocess.CalledProcessError as ex:
            os.chdir(pwd)
            raise ex

        make_install = ['ninja', 'install']
        saver.update_progress(20.0)
        try:
            run_command_cb(make_install, saver.process)
        except subprocess.CalledProcessError as ex:
            os.chdir(pwd)
            raise ex

        saver.update_progress(70.0)
        in_file = open('CPackConfig.cmake', 'r')
        for line in in_file.readlines():
            res = re.search(r'SET\(CPACK_PACKAGE_FILE_NAME "(.+)"\)', line)
            if res != None:
                filename = res.group(1)
                break
        in_file.close()

        saver.update_progress(80.0)
        file_names = []
        if self.platform_.name() == 'android':
            make_apk_release = ['ninja', 'apk_release']
            try:
                run_command_cb(make_apk_release, saver.process)
            except subprocess.CalledProcessError as ex:
                os.chdir(pwd)
                raise ex
            make_apk_signed = ['ninja', 'apk_signed']
            try:
                run_command_cb(make_apk_signed, saver.process)
            except subprocess.CalledProcessError as ex:
                os.chdir(pwd)
                raise ex
            make_apk_signed_aligned = ['ninja', 'apk_signed_aligned']
            try:
                run_command_cb(make_apk_signed_aligned, saver.process)
            except subprocess.CalledProcessError as ex:
                os.chdir(pwd)
                raise ex

            file_names.append(os.path.join(abs_dir_path, filename + '.' + system_info.get_extension_by_package('APK')))
        else:
            for generator in package_types:
                make_cpack = ['cpack', '-G', generator]
                try:
                    run_command_cb(make_cpack, saver.process)
                    file_names.append(os.path.join(abs_dir_path, filename + '.' + system_info.get_extension_by_package(generator)))
                except subprocess.CalledProcessError as ex:
                    os.chdir(pwd)
                    raise ex

        os.chdir(pwd)
        saver.process("Building finished successfully file_names: {0}".format(file_names))

        saver.update_progress(100.0)
        return file_names

if __name__ == "__main__":
    argc = len(sys.argv)

    if argc > 1:
        cmake_root = sys.argv[1]
    else:
        print_usage()
        sys.exit(1)

    if argc > 2:
        branding_file_path = sys.argv[2]
    else:
        branding_file_path = '/dev/null'
        
    if argc > 3:
        platform_str = sys.argv[3]
    else:
        platform_str = system_info.get_os()

    if argc > 4:
        arch_bit_str = sys.argv[4]
    else:
        arch_bit_str = system_info.get_arch_bit()

    if argc > 5:
        packages = sys.argv[5].split()
    else:
        packages = []
        
    request = BuildRequest(platform_str, int(arch_bit_str))
    abs_branding_file = os.path.abspath(branding_file_path)
    branding_options = read_file_line_by_line(abs_branding_file)
    saver = ProgressSaverBase()
    request.build(cmake_root, branding_options, 'build_' + platform_str, packages, saver)
