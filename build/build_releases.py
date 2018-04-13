#!/usr/bin/env python3
import os
import sys

from datetime import datetime

import build
from pybuild_utils.base import system_info
from pybuild_utils.base import utils


def print_usage():
    print("Usage:\n"
          "[required] argv[1] cmake_root_path\n"
          "[optional] argv[2] platform\n"
          "[optional] argv[3] architecture\n"
          "[optional] argv[4] build system(\"ninja\", \"make\")\n"
          "[optional] argv[5] packages for example(\"DEB RPM\")\n")


def print_message(progress, message):
    print('{0:.1f}% {1}'.format(progress, message))
    sys.stdout.flush()


if __name__ == "__main__":
    argc = len(sys.argv)

    if argc > 1:
        cmake_root = sys.argv[1]
    else:
        print_usage()
        sys.exit(1)

    if argc > 2:
        platform_str = sys.argv[2]
    else:
        platform_str = system_info.get_os()

    if argc > 3:
        arch_bit_str = sys.argv[3]
    else:
        arch_bit_str = system_info.get_arch_name()

    if argc > 4:
        bs_str = sys.argv[4]
        bs = system_info.get_supported_build_system_by_name(bs_str)
    else:
        bs = None

    if argc > 5:
        packages = sys.argv[5].split()
    else:
        packages = []

    request = build.BuildRequest(platform_str, arch_bit_str)
    saver = build.ProgressSaver(print_message)

    # fastonosql build
    fastonosql_branding_file = os.path.abspath('fastonosql.txt')
    fastonosql_branding_options = utils.read_file_line_by_line_to_list(fastonosql_branding_file)
    # request.build(cmake_root, fastonosql_branding_options, 'build_fastonosql_' + platform_str, bs, packages, saver)

    request.build(cmake_root, fastonosql_branding_options, 'build_fastonosql_' + platform_str, bs,
                  packages, saver)

    # fastoredis build
    fastoredis_branding_file = os.path.abspath('fastoredis.txt')
    fastoredis_branding_options = utils.read_file_line_by_line_to_list(fastoredis_branding_file)
    # request.build(cmake_root, fastoredis_branding_options, 'build_fastoredis_' + platform_str, bs, packages, saver)

    request.build(cmake_root, fastoredis_branding_options, 'build_fastoredis_' + platform_str, bs,
                  packages, saver)
