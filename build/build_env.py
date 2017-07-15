#!/usr/bin/env python
import os
import shutil
import sys

from pybuild_utils.base import run_command
from pybuild_utils.base import system_info
from pybuild_utils.base import utils


def print_usage():
    print("Usage:\n"
          "[optional] argv[1] platform\n"
          "[optional] argv[2] architecture\n"
          "[optional] argv[3] build system for common/qscintilla/libssh2 (\"ninja\", \"make\", \"gmake\")\n"
          "[optional] argv[4] build system for leveldb/rocksdb/upscaledb (\"make\", \"gmake\")\n"
          "[optional] argv[5] prefix path\n")


def print_message(progress, message):
    print(message.message())
    sys.stdout.flush()


class BuildRequest(object):
    def __init__(self, platform, arch_bit):
        platform_or_none = system_info.get_supported_platform_by_name(platform)

        if not platform_or_none:
            raise utils.BuildError('invalid platform')

        arch = platform_or_none.architecture_by_arch_name(arch_bit)
        if arch == None:
            raise utils.BuildError('invalid arch')

        self.platform_ = platform_or_none.make_platform_by_arch(arch, platform_or_none.package_types())
        print("Build request for platform: {0}, arch: {1} created".format(platform, arch.name()))

    def build(self, dir_path, bs, bs_external, prefix_path):
        cmake_project_root_abs_path = '..'
        if not os.path.exists(cmake_project_root_abs_path):
            raise utils.BuildError('invalid cmake_project_root_path: %s' % cmake_project_root_abs_path)

        if not bs:
            bs = system_info.SUPPORTED_BUILD_SYSTEMS[0]

        if not bs_external:
            bs_external = system_info.SUPPORTED_BUILD_SYSTEMS[1]

        if prefix_path == None:
            prefix_path = self.platform_.arch().default_install_prefix_path()

        abs_dir_path = os.path.abspath(dir_path)
        if os.path.exists(abs_dir_path):
            shutil.rmtree(abs_dir_path)

        is_android = self.platform_.name() == 'android'

        generator = bs.cmake_generator_arg()
        build_system_args = bs.cmd_line()
        # bs_name = bs.name()

        pwd = os.getcwd()
        os.mkdir(abs_dir_path)
        os.chdir(abs_dir_path)

        # project static options
        prefix_args = '-DCMAKE_INSTALL_PREFIX={0}'.format(prefix_path)

        cmake_line = ['cmake', cmake_project_root_abs_path, generator, '-DCMAKE_BUILD_TYPE=RELEASE', prefix_args]

        if is_android:
            toolchain_path = os.path.join(cmake_project_root_abs_path, 'cmake/android.toolchain.cmake')
            cmake_line.append('-DCMAKE_TOOLCHAIN_FILE={0}'.format(toolchain_path))

        make_install = build_system_args
        make_install.append('install')

        try:
            cloned_dir = utils.git_clone('https://github.com/fastogt/common.git', abs_dir_path)
            os.chdir(cloned_dir)

            os.mkdir('build_cmake_release')
            os.chdir('build_cmake_release')
            common_cmake_line = list(cmake_line)
            common_cmake_line.append('-DQT_ENABLED=ON')
            cmake_policy = run_command.CmakePolicy(print_message)
            make_policy = run_command.CommonPolicy(print_message)
            run_command.run_command_cb(common_cmake_line, cmake_policy)
            run_command.run_command_cb(make_install, make_policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        try:
            cloned_dir = utils.git_clone('https://github.com/fastogt/qscintilla.git', abs_dir_path)
            qsci_src_path = os.path.join(cloned_dir, 'Qt4Qt5')
            os.chdir(qsci_src_path)

            os.mkdir('build_cmake_release')
            os.chdir('build_cmake_release')
            qscintilla_cmake_line = list(cmake_line)
            cmake_policy = run_command.CmakePolicy(print_message)
            make_policy = run_command.CommonPolicy(print_message)
            run_command.run_command_cb(qscintilla_cmake_line, cmake_policy)
            run_command.run_command_cb(make_install, make_policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        try:
            cloned_dir = utils.git_clone('https://github.com/fastogt/libssh2.git', abs_dir_path)
            os.chdir(cloned_dir)

            os.mkdir('build_cmake_release')
            os.chdir('build_cmake_release')
            libssh2_cmake_line = list(cmake_line)
            libssh2_cmake_line.append('-DBUILD_SHARED_LIBS=OFF')
            libssh2_cmake_line.append('-DCRYPTO_BACKEND=OpenSSL')
            libssh2_cmake_line.append('-DENABLE_ZLIB_COMPRESSION=ON')
            libssh2_cmake_line.append('-DZLIB_USE_STATIC=ON')
            libssh2_cmake_line.append('-DOPENSSL_USE_STATIC=ON')
            cmake_policy = run_command.CmakePolicy(print_message)
            make_policy = run_command.CommonPolicy(print_message)
            run_command.run_command_cb(libssh2_cmake_line, cmake_policy)
            run_command.run_command_cb(make_install, make_policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        if is_android:
            return

        build_external_system_args = bs_external.cmd_line()

        try:
            cloned_dir = utils.git_clone('https://github.com/fastogt/leveldb.git', abs_dir_path)
            os.chdir(cloned_dir)

            make_leveldb = list(build_external_system_args)
            make_policy = run_command.CommonPolicy(print_message)
            run_command.run_command_cb(make_leveldb, make_policy)

            copy_leveldb_includes = ['cp', '-r', 'include/leveldb', '{0}/include'.format(prefix_path)]
            copy_policy = run_command.CommonPolicy(print_message)
            run_command.run_command_cb(copy_leveldb_includes, copy_policy)

            copy_leveldb_libs = ['cp', 'out-static/libleveldb.a', '{0}/lib'.format(prefix_path)]
            run_command.run_command_cb(copy_leveldb_libs, copy_policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        try:
            cloned_dir = utils.git_clone('https://github.com/fastogt/rocksdb.git', abs_dir_path)
            os.chdir(cloned_dir)

            os.mkdir('build_cmake_release')
            os.chdir('build_cmake_release')
            common_cmake_line = list(cmake_line)
            #common_cmake_line.append('-DROCKSDB_LITE=ON')
            cmake_policy = run_command.CmakePolicy(print_message)
            make_policy = run_command.CommonPolicy(print_message)
            run_command.run_command_cb(common_cmake_line, cmake_policy)
            run_command.run_command_cb(make_install, make_policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        try:
            cloned_dir = utils.git_clone('https://github.com/fastogt/upscaledb.git', abs_dir_path)
            os.chdir(cloned_dir)
            bootstrap_policy = run_command.CommonPolicy(print_message)
            bootstrap_upscaledb = ['sh', 'bootstrap.sh']
            run_command.run_command_cb(bootstrap_upscaledb, bootstrap_policy)

            configure_upscaledb = ['./configure', '--prefix={0}'.format(prefix_path), '--disable-remote',
                                   '--enable-static-boost', '--disable-shared', '--disable-java',
                                   '--disable-encryption']
            configure_policy = run_command.CommonPolicy(print_message)
            run_command.run_command_cb(configure_upscaledb, configure_policy)

            make_install_upscaledb = list(build_external_system_args)
            make_install_upscaledb.append('install')
            make_policy = run_command.CommonPolicy(print_message)
            run_command.run_command_cb(make_install_upscaledb, make_policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex


if __name__ == "__main__":
    argc = len(sys.argv)

    if argc > 1:
        platform_str = sys.argv[1]
    else:
        platform_str = system_info.get_os()

    if argc > 2:
        arch_bit_str = sys.argv[2]
    else:
        arch_bit_str = system_info.get_arch_name()

    if argc > 3:
        bs_str = sys.argv[3]
        bs = system_info.get_supported_build_system_by_name(bs_str)
    else:
        bs = None

    if argc > 4:
        bs_external_str = sys.argv[4]
        bs_external = system_info.get_supported_build_system_by_name(bs_external_str)
    else:
        bs_external = None

    if argc > 5:
        prefix_path = sys.argv[5]
    else:
        prefix_path = None

    request = BuildRequest(platform_str, arch_bit_str)
    request.build('build_' + platform_str + '_env', bs, bs_external, prefix_path)
