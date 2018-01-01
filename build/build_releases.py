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
          "[required] argv[2] expired utc timestamp\n"
          "[optional] argv[3] platform\n"
          "[optional] argv[4] architecture\n"
          "[optional] argv[5] build system(\"ninja\", \"make\")\n"
          "[optional] argv[6] packages for example(\"DEB RPM\")\n")


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
        expired_time_str = sys.argv[2]
        expired_time = datetime.utcfromtimestamp(int(expired_time_str))
        cur_utc = datetime.utcnow()
        if expired_time < cur_utc:
            print("Expired utc time({0}) should be greater than current utc time({1})".format(expired_time, cur_utc))
            print_usage()
            sys.exit(1)
    else:
        print_usage()
        sys.exit(1)

    if argc > 3:
        platform_str = sys.argv[3]
    else:
        platform_str = system_info.get_os()

    if argc > 4:
        arch_bit_str = sys.argv[4]
    else:
        arch_bit_str = system_info.get_arch_name()

    if argc > 5:
        bs_str = sys.argv[5]
        bs = system_info.get_supported_build_system_by_name(bs_str)
    else:
        bs = None

    if argc > 6:
        packages = sys.argv[6].split()
    else:
        packages = []

    request = build.BuildRequest(platform_str, arch_bit_str)
    saver = build.ProgressSaver(print_message)

    # fastonosql build
    fastonosql_branding_file = os.path.abspath('fastonosql.txt')
    fastonosql_branding_options = utils.read_file_line_by_line_to_list(fastonosql_branding_file)
    # request.build(cmake_root, fastonosql_branding_options, 'build_fastonosql_' + platform_str, bs, packages, saver)

    fastonosql_branding_options_trial = fastonosql_branding_options
    fastonosql_branding_options_trial.append('-DEXPIRE_APPLICATION_UTC_TIME={0}'.format(expired_time_str))
    request.build(cmake_root, fastonosql_branding_options_trial, 'build_fastonosql_' + platform_str + '_trial', bs,
                  packages, saver)

    # fastoredis build
    fastoredis_branding_file = os.path.abspath('fastoredis.txt')
    fastoredis_branding_options = utils.read_file_line_by_line_to_list(fastoredis_branding_file)
    # request.build(cmake_root, fastoredis_branding_options, 'build_fastoredis_' + platform_str, bs, packages, saver)

    fastoredis_branding_options_trial = fastoredis_branding_options
    fastoredis_branding_options_trial.append('-DEXPIRE_APPLICATION_UTC_TIME={0}'.format(expired_time_str))
    request.build(cmake_root, fastoredis_branding_options_trial, 'build_fastoredis_' + platform_str + '_trial', bs,
                  packages, saver)
