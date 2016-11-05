#!/usr/bin/env python
import sys
import os
import shutil
from base import system_info
from base import run_command

def print_usage():
    print("Usage:\n"
        "[optional] argv[1] platform\n"
        "[optional] argv[2] architecture\n"
        "[optional] argv[3] build system(\"ninja\", \"make\")\n"
        "[optional] argv[4] prefix path\n")

def stable_path(path):
    return path.replace("\\", "/")

def print_message(progress, message):
    print message.message()
    sys.stdout.flush()

class BuildError(Exception):
    def __init__(self, value):
        self.value_ = value
    def __str__(self):
        return self.value_

class BuildSystem:
    def __init__(self, name, cmd_line, cmake_generator_arg):
        self.name_ = name
        self.cmd_line_ = cmd_line
        self.cmake_generator_arg_ = cmake_generator_arg

    def cmake_generator_arg(self):
        return self.cmake_generator_arg_

    def name(self):
        return self.name_

    def cmd_line(self):  # cmd + args
        return self.cmd_line_

SUPPORTED_BUILD_SYSTEMS = [BuildSystem('ninja', ['ninja'], '-GNinja'),
                               BuildSystem('make', ['make', '-j2'], '-GUnix Makefiles')]

def get_supported_build_system_by_name(name):
    return next((x for x in SUPPORTED_BUILD_SYSTEMS if x.name() == name), None)

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

    def build(self, dir_path, bs, prefix_path):
        cmake_project_root_abs_path = '..'
        if not os.path.exists(cmake_project_root_abs_path):
            raise BuildError('invalid cmake_project_root_path: %s' % cmake_project_root_abs_path)

        if not bs:
            bs = SUPPORTED_BUILD_SYSTEMS[0]

        if prefix_path == None:
            prefix_path = self.platform_.arch().default_install_prefix_path()

        #prefix_path = stable_path(os.path.abspath(prefix_path))
        abs_dir_path = os.path.abspath(dir_path)
        if os.path.exists(abs_dir_path):
            shutil.rmtree(abs_dir_path)

        is_android = self.platform_.name() == 'android'

        generator = bs.cmake_generator_arg()
        build_system_args = bs.cmd_line()

        pwd = os.getcwd()
        os.mkdir(abs_dir_path)
        os.chdir(abs_dir_path)

        # project static options
        arch = self.platform_.arch()
        arch_args = '-DOS_ARCH={0}'.format(arch.bit())
        prefix_args = '-DCMAKE_INSTALL_PREFIX={0}'.format(prefix_path)

        cmake_line = ['cmake', cmake_project_root_abs_path, generator, '-DCMAKE_BUILD_TYPE=RELEASE', arch_args,
                      prefix_args]

        if is_android:
            toolchain_path = os.path.join(cmake_project_root_abs_path, 'cmake/android.toolchain.cmake')
            cmake_line.append('-DCMAKE_TOOLCHAIN_FILE={0}'.format(toolchain_path))

        make_install = build_system_args
        make_install.append('install')

        try:
            policy = run_command.Policy(print_message)
            cloned_dir = self.git_clone('https://github.com/fastogt/common.git', policy)
            os.chdir(cloned_dir)

            os.mkdir('build_cmake_release')
            os.chdir('build_cmake_release')
            common_cmake_line = cmake_line
            common_cmake_line.append('-DQT_ENABLED=ON')
            run_command.run_command_cb(common_cmake_line, policy)
            run_command.run_command_cb(make_install, policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        try:
            policy = run_command.Policy(print_message)
            cloned_dir = self.git_clone('https://github.com/fastogt/qscintilla.git', policy)
            qsci_src_path = os.path.join(cloned_dir, 'Qt4Qt5')
            os.chdir(qsci_src_path)

            os.mkdir('build_cmake_release')
            os.chdir('build_cmake_release')
            qscintilla_cmake_line = cmake_line
            run_command.run_command_cb(qscintilla_cmake_line, policy)
            run_command.run_command_cb(make_install, policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        if is_android:
          return

        try:
            policy = run_command.Policy(print_message)
            cloned_dir = self.git_clone('https://github.com/fastogt/rocksdb.git', policy)
            os.chdir(cloned_dir)

            make_install_rocksdb = ['make', 'install-static']
            make_install_rocksdb.insert(0, 'INSTALL_PATH={0}'.format(prefix_path))
            make_install_rocksdb.insert(0, 'env')
            run_command.run_command_cb(make_install_rocksdb, policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

        try:
            policy = run_command.Policy(print_message)
            cloned_dir = self.git_clone('https://github.com/fastogt/upscaledb.git', policy)
            os.chdir(cloned_dir)

            bootstrap_upscaledb = ['sh', 'bootstrap.sh']
            run_command.run_command_cb(bootstrap_upscaledb, policy)

            configure_upscaledb = ['./configure', '--prefix={0}'.format(prefix_path), '--disable-remote']
            run_command.run_command_cb(configure_upscaledb, policy)

            make_install_upscaledb = ['make', 'install']
            run_command.run_command_cb(make_install_upscaledb, policy)
            os.chdir(abs_dir_path)
        except Exception as ex:
            os.chdir(pwd)
            raise ex

    def git_clone(self, url, policy):
        pwd = os.getcwd()
        common_git_clone_line = ['git', 'clone']
        common_git_clone_line.append(url)
        cloned_dir = os.path.splitext(url.rsplit('/', 1)[-1])[0]
        common_git_clone_line.append(cloned_dir)
        run_command.run_command_cb(common_git_clone_line, policy)
        os.chdir(cloned_dir)

        common_git_clone_init_line = ['git', 'submodule', 'update', '--init', '--recursive']
        run_command.run_command_cb(common_git_clone_init_line, policy)
        os.chdir(pwd)
        return os.path.join(pwd, cloned_dir)

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
        bs_str = sys.argv[3]
        bs = get_supported_build_system_by_name(bs_str)
    else:
        bs = []

    if argc > 4:
        prefix_path = sys.argv[4]
    else:
        prefix_path = None

    request = BuildRequest(platform_str, int(arch_bit_str))
    request.build('build_' + platform_str, bs, prefix_path)
