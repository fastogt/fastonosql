#!/usr/bin/env python
import os
import re
import shutil
import sys

from pybuild_utils.base import run_command
from pybuild_utils.base import system_info
from pybuild_utils.base import utils


def print_usage():
    print("Usage:\n"
          "[required] argv[1] cmake_root_path\n"
          "[optional] argv[2] branding_file_path\n"
          "[optional] argv[3] platform\n"
          "[optional] argv[4] architecture\n"
          "[optional] argv[5] build system(\"ninja\", \"make\", \"gmake\")\n"
          "[optional] argv[6] packages for example(\"DEB RPM\")\n")


class BuildSystem:
    def __init__(self, name, cmd_line, cmake_generator_arg, policy):
        self.name_ = name
        self.cmd_line_ = cmd_line
        self.cmake_generator_arg_ = cmake_generator_arg
        self.policy_ = policy

    def cmake_generator_arg(self) -> str:
        return self.cmake_generator_arg_

    def name(self) -> str:
        return self.name_

    def policy(self) -> run_command.Policy:
        return self.policy_

    def cmd_line(self) -> list:  # cmd + args
        return self.cmd_line_


SUPPORTED_BUILD_SYSTEMS = [BuildSystem('ninja', ['ninja'], '-GNinja', run_command.NinjaPolicy),
                           BuildSystem('make', ['make', '-j2'], '-GUnix Makefiles', run_command.MakePolicy)]


def get_supported_build_system_by_name(name):
    return next((x for x in SUPPORTED_BUILD_SYSTEMS if x.name() == name), None)


def print_message(progress, message):
    print('{0:.1f}% {1}'.format(progress, message))
    sys.stdout.flush()


class ProgressSaver(object):
    def __init__(self, cb):
        self.progress_min_ = 0.0
        self.progress_max_ = 0.0
        self.cb_ = cb

    def update_progress_message_range(self, progress_min, progress_max, message):
        self.progress_min_ = progress_min
        self.progress_max_ = progress_max
        if self.cb_:
            self.cb_(progress_min, message)

    def on_update_progress_message(self, progress, message):
        if message.type() == run_command.MessageType.STATUS:
            return

        diff = self.progress_max_ - self.progress_min_
        perc = self.progress_min_ + diff * (progress / 100.0)
        if self.cb_:
            self.cb_(perc, message.message())


class BuildRequest(object):
    def __init__(self, platform, arch_name):
        platform_or_none = system_info.get_supported_platform_by_name(platform)

        if not platform_or_none:
            raise utils.BuildError('invalid platform')

        arch = platform_or_none.architecture_by_arch_name(arch_name)
        if not arch:
            raise utils.BuildError('invalid arch')

        self.platform_ = platform_or_none.make_platform_by_arch(arch, platform_or_none.package_types())
        print("Build request for platform: {0}, arch: {1} created".format(platform, arch.name()))

    def platform(self):
        return self.platform_

    def build(self, cmake_project_root_path, app_branding_options, dir_path, bs, package_types, saver):
        cmake_project_root_abs_path = os.path.abspath(cmake_project_root_path)
        if not os.path.exists(cmake_project_root_abs_path):
            raise utils.BuildError('invalid cmake_project_root_path: %s' % cmake_project_root_path)

        if not bs:
            bs = SUPPORTED_BUILD_SYSTEMS[0]

        if not package_types:
            package_types = self.platform_.package_types()

        abs_dir_path = os.path.abspath(dir_path)
        if os.path.exists(abs_dir_path):
            shutil.rmtree(abs_dir_path)

        is_android = self.platform_.name() == 'android'

        generator = bs.cmake_generator_arg()
        build_system_args = bs.cmd_line()
        build_system_policy = bs.policy()

        saver.update_progress_message_range(0.0, 9.0, "Start building project branding_options:\n{0}".format(
            "\n".join(app_branding_options)))

        pwd = os.getcwd()
        os.mkdir(abs_dir_path)
        os.chdir(abs_dir_path)

        # project static options
        log_to_file_args = '-DLOG_TO_FILE=ON'
        if is_android:
            openssl_args = ['-DOPENSSL_USE_STATIC_LIBS=OFF']
            zlib_args = '-DZLIB_USE_STATIC=OFF'
            bzip2_args = '-DBZIP2_USE_STATIC=OFF'
        else:
            prefix_path = self.platform_.arch().default_install_prefix_path()
            openssl_args = ['-DOPENSSL_USE_STATIC_LIBS=ON', '-DOPENSSL_ROOT_DIR={0}'.format(prefix_path)]
            zlib_args = '-DZLIB_USE_STATIC=ON'
            bzip2_args = '-DBZIP2_USE_STATIC=ON'
        snappy_args = '-DSNAPPY_USE_STATIC=ON'
        jsonc_args = '-DJSONC_USE_STATIC=ON'

        cmake_line = ['cmake', cmake_project_root_abs_path, generator, '-DCMAKE_BUILD_TYPE=RELEASE', log_to_file_args,
                      zlib_args, bzip2_args, snappy_args, jsonc_args]
        cmake_line.extend(openssl_args)

        if is_android:
            toolchain_path = os.path.join(cmake_project_root_abs_path, 'cmake/android.toolchain.cmake')
            cmake_line.append('-DCMAKE_TOOLCHAIN_FILE={0}'.format(toolchain_path))

        if app_branding_options:
            cmake_line.extend(app_branding_options)

        saver.update_progress_message_range(10.0, 19.0, 'Generate project build')

        def store(cb):
            def closure(progress, message):
                return cb(progress, message)

            return closure

        store = store(saver.on_update_progress_message)

        try:
            cmake_policy = run_command.CmakePolicy(store)
            run_command.run_command_cb(cmake_line, cmake_policy)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        make_install = build_system_args
        make_install.append('install')
        saver.update_progress_message_range(20.0, 79.0, 'Build project')
        try:
            policy = build_system_policy(store)
            run_command.run_command_cb(make_install, policy)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        saver.update_progress_message_range(80.0, 84.0, 'Trying to get package file name')
        in_file = open('CPackConfig.cmake', 'r')
        for line in in_file.readlines():
            res = re.search(r'SET\(CPACK_PACKAGE_FILE_NAME "(.+)"\)', line)
            if res:
                filename = res.group(1)
                break
        in_file.close()

        saver.update_progress_message_range(85.0, 99.0, 'Start build package')
        file_names = []
        if is_android:
            make_apk_release = build_system_args
            make_apk_release.append('apk_release')
            try:
                common_policy = run_command.CommonPolicy(store)
                run_command.run_command_cb(make_apk_release, common_policy)
            except Exception as ex:
                os.chdir(pwd)
                raise ex

            make_apk_aligned = build_system_args
            make_apk_aligned.append('apk_aligned')
            try:
                common_policy = run_command.CommonPolicy(store)
                run_command.run_command_cb(make_apk_aligned, common_policy)
            except Exception as ex:
                os.chdir(pwd)
                raise ex

            make_apk_signed = build_system_args
            make_apk_signed('apk_signed')
            try:
                common_policy = run_command.CommonPolicy(store)
                run_command.run_command_cb(make_apk_signed, common_policy)
            except Exception as ex:
                os.chdir(pwd)
                raise ex

            file_names.append(os.path.join(abs_dir_path, filename + '.' + system_info.get_extension_by_package('APK')))
        else:
            for generator in package_types:
                make_cpack = ['cpack', '-G', generator]
                try:
                    common_policy = run_command.CommonPolicy(store)
                    run_command.run_command_cb(make_cpack, common_policy)
                    file_names.append(
                        os.path.join(abs_dir_path, filename + '.' + system_info.get_extension_by_package(generator)))
                except Exception as ex:
                    os.chdir(pwd)
                    raise ex

        os.chdir(pwd)

        saver.update_progress_message_range(100.0, 100.0,
                                            "Building finished successfully file_names: {0}".format(file_names))
        return file_names


if __name__ == "__main__":
    argc = len(sys.argv)
    dev_null = '/dev/null'

    if argc > 1:
        cmake_root = sys.argv[1]
    else:
        print_usage()
        sys.exit(1)

    if argc > 2:
        branding_file_path = sys.argv[2]
    else:
        branding_file_path = dev_null

    if argc > 3:
        platform_str = sys.argv[3]
    else:
        platform_str = system_info.get_os()

    if argc > 4:
        arch_name_str = sys.argv[4]
    else:
        arch_name_str = system_info.get_arch_name()

    if argc > 5:
        bs_str = sys.argv[5]
        build_system = get_supported_build_system_by_name(bs_str)
    else:
        build_system = []

    if argc > 6:
        packages = sys.argv[6].split()
    else:
        packages = []

    request = BuildRequest(platform_str, arch_name_str)
    if branding_file_path != dev_null:
        abs_branding_file = os.path.abspath(branding_file_path)
        branding_options = utils.read_file_line_by_line(abs_branding_file)
    else:
        branding_options = []

    saver = ProgressSaver(print_message)
    request.build(cmake_root, branding_options, 'build_' + platform_str, build_system, packages, saver)
