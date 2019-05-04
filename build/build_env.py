#!/usr/bin/env python3
import os
import shutil
import sys
import subprocess

from pyfastogt import system_info
from pyfastogt import utils

OPENSSL_SRC_ROOT = "https://www.openssl.org/source/"
ARCH_OPENSSL_COMP = "gz"
ARCH_OPENSSL_EXT = "tar." + ARCH_OPENSSL_COMP
# ROCKSDB_BRANCH = 'v5.10.2'
g_script_path = os.path.realpath(sys.argv[0])


def print_usage():
    print("Usage:\n"
          "[optional] argv[1] platform\n"
          "[optional] argv[2] architecture\n"
          "[optional] argv[3] build system for common/qscintilla/libssh2 (\"ninja\", \"make\", \"gmake\")\n")


class BuildRequest(object):
    def __init__(self, platform, arch_bit, dir_path):
        platform_or_none = system_info.get_supported_platform_by_name(platform)

        if not platform_or_none:
            raise utils.BuildError('invalid platform')

        arch = platform_or_none.architecture_by_arch_name(arch_bit)
        if not arch:
            raise utils.BuildError('invalid arch')

        build_dir_path = os.path.abspath(dir_path)
        if os.path.exists(build_dir_path):
            shutil.rmtree(build_dir_path)

        os.mkdir(build_dir_path)
        os.chdir(build_dir_path)

        self.build_dir_path_ = build_dir_path

        self.platform_ = platform_or_none.make_platform_by_arch(arch, platform_or_none.package_types())
        self.prefix_path_ = self.platform_.arch().default_install_prefix_path()
        print("Build request for platform: {0}, arch: {1} created".format(platform, arch.name()))

    def __get_system_libs(self):
        platform = self.platform_
        platform_name = platform.name()
        arch = platform.arch()
        dep_libs = []

        if platform_name == 'linux':
            distribution = system_info.linux_get_dist()
            if distribution == 'DEBIAN':
                dep_libs = ['git', 'gcc', 'g++', 'yasm', 'pkg-config', 'libtool', 'rpm',
                            'autogen', 'autoconf',
                            'cmake', 'make', 'ninja-build',
                            'libz-dev', 'libbz2-dev', 'lz4-dev',
                            'qt5']
            elif distribution == 'RHEL':
                dep_libs = ['git', 'gcc', 'gcc-c++', 'yasm', 'pkgconfig', 'libtool', 'rpm-build',
                            'autogen', 'autoconf',
                            'cmake', 'make', 'ninja-build',
                            'zlib-devel', 'bzip2-devel', 'lz4-devel',
                            'qt5-qtbase-devel', 'qt5-linguist']
        elif platform_name == 'windows':
            if arch.name() == 'x86_64':
                dep_libs = ['git', 'make', 'mingw-w64-x86_64-gcc', 'mingw-w64-x86_64-yasm',
                            'mingw-w64-x86_64-ninja', 'mingw-w64-x86_64-make', 'mingw-w64-x86_64-cmake',
                            'mingw-w64-x86_64-qt5']
            elif arch.name() == 'i386':
                dep_libs = ['git', 'make', 'mingw-w64-i686-gcc', 'mingw-w64-i686-yasm',
                            'mingw-w64-i686-ninja', 'mingw-w64-i686-make', 'mingw-w64-i686-cmake',
                            'mingw-w64-i686-qt5']
        elif platform_name == 'macosx':
            dep_libs = ['git', 'yasm', 'make', 'ninja', 'cmake', 'qt5']
        else:
            raise NotImplemented("Unknown platform '%s'" % platform_name)

        return dep_libs

    def build_system(self):
        platform = self.platform_
        dep_libs = self.__get_system_libs()
        for lib in dep_libs:
            platform.install_package(lib)

        # post install step

    def build_snappy(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/snappy.git')
        self.__build_via_cmake(cloned_dir, ['-DBUILD_SHARED_LIBS=OFF', '-DSNAPPY_BUILD_TESTS=OFF'])

    def build_common(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/common.git')
        self.__build_via_cmake(cloned_dir, ['-DQT_ENABLED=ON'])

    def build_openssl(self, version):
        compiler_flags = utils.CompileInfo([], ['no-shared', 'no-unit-test'])
        url = '{0}openssl-{1}.{2}'.format(OPENSSL_SRC_ROOT, version, ARCH_OPENSSL_EXT)
        self.__download_and_build_via_configure(url, compiler_flags, './config')

    def build_libssh2(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/libssh2.git')
        self.__build_via_cmake(cloned_dir,
                               ['-DBUILD_SHARED_LIBS=OFF', '-DCRYPTO_BACKEND=OpenSSL', '-DENABLE_ZLIB_COMPRESSION=ON',
                                '-DBUILD_EXAMPLES=OFF', '-DBUILD_TESTING=OFF', '-DOPENSSL_USE_STATIC_LIBS=ON',
                                '-DZLIB_USE_STATIC=ON', '-DOPENSSL_ROOT_DIR={0}'.format(self.prefix_path_)])

    def build_jsonc(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/json-c.git')
        self.__build_via_cmake(cloned_dir, ['-DBUILD_SHARED_LIBS=OFF'])

    def build_qscintilla(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/qscintilla.git')
        qsci_src_path = os.path.join(cloned_dir, 'Qt4Qt5')
        self.__build_via_cmake(qsci_src_path)

    def build_hiredis(self):
        try:
            cloned_dir = utils.git_clone('git@github.com:fastogt/hiredis.git')
            os.chdir(cloned_dir)

            make_hiredis = ['make', 'LIBSSH2_ENABLED=ON', 'OPENSSL_ROOT_DIR={0}'.format(self.prefix_path_),
                            'PREFIX={0}'.format(self.prefix_path_), 'install']
            subprocess.call(make_hiredis)
        except Exception as ex:
            raise ex
        finally:
            os.chdir(self.build_dir_path_)

    def build_libmemcached(self):
        try:
            cloned_dir = utils.git_clone('git@github.com:fastogt/libmemcached.git')
            os.chdir(cloned_dir)

            bootstrap_libmemcached = ['sh', 'bootstrap.sh']
            subprocess.call(bootstrap_libmemcached)

            self.__build_via_configure(utils.CompileInfo([], ['--disable-shared', '--enable-static', '--enable-sasl']))
        except Exception as ex:
            raise ex
        finally:
            os.chdir(self.build_dir_path_)

    def build_unqlite(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/unqlite.git')
        self.__build_via_cmake(cloned_dir)

    def build_lmdb(self):
        try:
            cloned_dir = utils.git_clone('git@github.com:fastogt/lmdb.git')
            os.chdir(cloned_dir)

            os.chdir('libraries/liblmdb')
            make_lmdb = ['make', 'install_static_lib', 'prefix={0}'.format(self.prefix_path_)]  # FIXME
            subprocess.call(make_lmdb)
        except Exception as ex:
            raise ex
        finally:
            os.chdir(self.build_dir_path_)

    def build_leveldb(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/leveldb.git')
        self.__build_via_cmake(cloned_dir, ['-DBUILD_SHARED_LIBS=OFF', '-DLEVELDB_BUILD_TESTS=OFF',
                                            '-DLEVELDB_BUILD_BENCHMARKS=OFF'])

    def build_rocksdb(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/rocksdb.git')
        self.__build_via_cmake(cloned_dir, ['-DFAIL_ON_WARNINGS=OFF', '-DPORTABLE=ON',
                                            '-DWITH_TESTS=OFF', '-DWITH_SNAPPY=ON', '-DWITH_ZLIB=ON', '-DWITH_LZ4=ON',
                                            '-DROCKSDB_INSTALL_ON_WINDOWS=ON', '-DWITH_TOOLS=OFF', '-DWITH_GFLAGS=OFF',
                                            '-DBUILD_SHARED_LIBS=OFF'])

    def build_forestdb(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/forestdb.git', None, False)
        self.__build_via_cmake(cloned_dir, ['-DBUILD_SHARED_LIBS=OFF'])

    def build_fastonosql_core(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/fastonosql_core.git', None, False)
        self.__build_via_cmake(cloned_dir, ['-DJSONC_USE_STATIC=ON', '-DOPENSSL_USE_STATIC_LIBS=ON'])

    def __build_via_cmake(self, directory, cmake_flags=[]):
        utils.build_command_cmake(directory, self.prefix_path_, cmake_flags)

    def __download_and_build_via_configure(self, url, compiler_flags: utils.CompileInfo, executable='./configure'):
        utils.build_from_sources(url, compiler_flags, g_script_path, self.prefix_path_, executable)

    def __build_via_configure(self, compiler_flags: utils.CompileInfo, executable='./configure'):
        utils.build_command_configure(compiler_flags, g_script_path, self.prefix_path_, executable)

    def build(self):
        # self.build_system()
        self.build_snappy()
        self.build_openssl('1.1.1b')  #
        self.build_libssh2()
        self.build_jsonc()
        self.build_qscintilla()
        self.build_common()

        # databases libs builds
        self.build_hiredis()
        self.build_libmemcached()  #
        self.build_unqlite()
        self.build_lmdb()
        self.build_leveldb()
        self.build_rocksdb()
        self.build_forestdb()  #
        self.build_fastonosql_core()


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

    request = BuildRequest(platform_str, arch_bit_str, 'build_' + platform_str + '_env')
    request.build()
