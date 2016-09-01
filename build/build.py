#!/usr/bin/env python
import subprocess
import sys
import os
import shutil
from base import system_info

class BuildError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return self.value

def print_usage():
    print("Usage:\n"
        "[optional] argv[1] platform\n"
        "[optional] argv[2] architecture\n"
        "[optional] argv[3] branding_file_path\n")

def run_command(cmd):
    return subprocess.check_call(cmd)


class BuildRequest(object):
    def __init__(self, platform, arch_bit):
        platform_or_none = system_info.get_supported_platform_by_name(platform)

        if platform_or_none == None:
            raise BuildError('invalid platform')

        arch = None
        for curr_arch in platform_or_none.archs:
            if (curr_arch.bit == arch_bit):
                arch = curr_arch
                break

        if arch == None:
            raise BuildError('invalid arch')

        self.platform = platform_or_none
        self.arch = arch
        print("Build request for platform {0}, arch {1} created.\n".format(platform, arch.name))

    def build(self, branding_file, dir_path, package_types):
        if not os.path.exists(branding_file):
            raise BuildError('invalid branding_file path')

        if not package_types:
            package_types = self.platform.package_types

        if os.path.exists(dir_path):
            shutil.rmtree(dir_path)

        print("Start building branding_file {0}, dir_path {1}.\n".format(branding_file, dir_path))

        branding_file_array = []
        with open(branding_file, "r") as ins:
            for line in ins:
                branding_file_array.append(line.strip('\n'))

        pwd = os.getcwd()
        os.mkdir(dir_path)
        os.chdir(dir_path)

        arch_args = '-DOS_ARCH={0}'.format(self.arch.bit)
        log_to_file_args = '-DLOG_TO_FILE=ON'
        openssl_args = '-DOPENSSL_USE_STATIC=ON'
        cmake_line = ['cmake', '../../', '-GNinja', '-DCMAKE_BUILD_TYPE=RELEASE', arch_args, log_to_file_args, openssl_args]

        if branding_file_array:
            cmake_line.extend(branding_file_array)

        try:
            run_command(cmake_line)
        except subprocess.CalledProcessError as ex:
            os.chdir(pwd)
            raise ex

        make_install = ['ninja', 'install']
        try:
            run_command(make_install)
        except subprocess.CalledProcessError as ex:
            os.chdir(pwd)
            raise ex

        if self.platform.name == 'android':
            make_apk_release = ['ninja', 'apk_release']
            try:
                run_command(make_apk_release)
            except subprocess.CalledProcessError as ex:
                os.chdir(pwd)
                raise ex
            make_apk_signed = ['ninja', 'apk_signed']
            try:
                run_command(make_apk_signed)
            except subprocess.CalledProcessError as ex:
                os.chdir(pwd)
                raise ex
            make_apk_signed_aligned = ['ninja', 'apk_signed_aligned']
            try:
                run_command(make_apk_signed_aligned)
            except subprocess.CalledProcessError as ex:
                os.chdir(pwd)
                raise ex
        else:
            for generator in package_types:
                make_cpack = ['cpack', '-G', generator]
                try:
                    run_command(make_cpack)
                except subprocess.CalledProcessError as ex:
                    os.chdir(pwd)
                    raise ex


        print("Building finished succsesfull.\n")

if __name__ == "__main__":
    argc = len(sys.argv)

    if argc > 1:
        platform_str = sys.argv[1]
    else:
        platform_str = system_info.get_os()

    if argc > 2:
        arch_bit_str = sys.argv[2]
    else:
        arch_bit_str = system_info.get_arch_bit()

    if argc > 3:
        branding_file_path = sys.argv[3]
    else:
        branding_file_path = '/dev/null'
        
    request = BuildRequest(platform_str, int(arch_bit_str))
    request.build(branding_file_path, platform_str + '_build', [])