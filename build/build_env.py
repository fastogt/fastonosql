#!/usr/bin/env python3
import os
import sys
import subprocess
import argparse

from pyfastogt import system_info
from pyfastogt import utils, build_utils

g_script_path = os.path.realpath(sys.argv[0])


def print_usage():
    print("Usage:\n"
          "[optional] argv[1] platform\n"
          "[optional] argv[2] architecture\n"
          "[optional] argv[3] build system for common/qscintilla/libssh2 (\"ninja\", \"make\", \"gmake\")\n")


class BuildRequest(build_utils.BuildRequest):
    def __init__(self, platform: str, arch: str, dir_path: str, prefix_path: str):
        patches_path = os.path.abspath(os.path.join(g_script_path, os.pardir))
        build_utils.BuildRequest.__init__(self, platform, arch, patches_path, dir_path, prefix_path)

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

    def build_libssh2(self):
        self._clone_and_build_via_cmake(build_utils.generate_fastogt_git_path('libssh2'),
                                        ['-DBUILD_SHARED_LIBS=OFF', '-DCRYPTO_BACKEND=OpenSSL',
                                         '-DENABLE_ZLIB_COMPRESSION=ON',
                                         '-DBUILD_EXAMPLES=OFF', '-DBUILD_TESTING=OFF', '-DOPENSSL_USE_STATIC_LIBS=ON',
                                         '-DZLIB_USE_STATIC=ON', '-DOPENSSL_ROOT_DIR={0}'.format(self.prefix_path_)])

    def build_qscintilla(self):
        cloned_dir = utils.git_clone('git@github.com:fastogt/qscintilla.git')
        qsci_src_path = os.path.join(cloned_dir, 'Qt4Qt5')
        os.chdir(qsci_src_path)
        self._build_via_cmake([])
        os.chdir(self.build_dir_path_)

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

            self._build_via_configure(
                build_utils.CompileInfo([], ['--disable-shared', '--enable-static', '--enable-sasl']))
        except Exception as ex:
            raise ex
        finally:
            os.chdir(self.build_dir_path_)

    def build_unqlite(self):
        self._clone_and_build_via_cmake(build_utils.generate_fastogt_git_path('unqlite'), [])

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
        self._clone_and_build_via_cmake(build_utils.generate_fastogt_git_path('leveldb'),
                                        ['-DBUILD_SHARED_LIBS=OFF', '-DLEVELDB_BUILD_TESTS=OFF',
                                         '-DLEVELDB_BUILD_BENCHMARKS=OFF'])

    def build_rocksdb(self):
        self._clone_and_build_via_cmake(build_utils.generate_fastogt_git_path('rocksdb'),
                                        ['-DFAIL_ON_WARNINGS=OFF', '-DPORTABLE=ON',
                                         '-DWITH_TESTS=OFF', '-DWITH_SNAPPY=ON', '-DWITH_ZLIB=ON', '-DWITH_LZ4=ON',
                                         '-DROCKSDB_INSTALL_ON_WINDOWS=ON', '-DWITH_TOOLS=OFF', '-DWITH_GFLAGS=OFF',
                                         '-DBUILD_SHARED_LIBS=OFF'])

    def build_forestdb(self):
        self._clone_and_build_via_cmake(build_utils.generate_fastogt_git_path('forestdb'), ['-DBUILD_SHARED_LIBS=OFF'])

    def build_fastonosql_core(self):
        self._clone_and_build_via_cmake(build_utils.generate_fastogt_git_path('fastonosql_core'),
                                        ['-DJSONC_USE_STATIC=ON', '-DOPENSSL_USE_STATIC_LIBS=ON'])


if __name__ == "__main__":
    openssl_default_version = '1.1.1b'

    host_os = system_info.get_os()
    arch_host_os = system_info.get_arch_name()

    parser = argparse.ArgumentParser(prog='build_env', usage='%(prog)s [options]')

    # system
    system_grp = parser.add_mutually_exclusive_group()
    system_grp.add_argument('--with-system', help='build with system dependencies (default)', dest='with_system',
                            action='store_true', default=True)
    system_grp.add_argument('--without-system', help='build without system dependencies', dest='with_system',
                            action='store_false', default=False)

    # snappy
    snappy_grp = parser.add_mutually_exclusive_group()
    snappy_grp.add_argument('--with-snappy', help='build snappy (default, version: git master)', dest='with_snappy',
                            action='store_true', default=True)
    snappy_grp.add_argument('--without-snappy', help='build without snappy', dest='with_snappy', action='store_false',
                            default=False)

    # json-c
    jsonc_grp = parser.add_mutually_exclusive_group()
    jsonc_grp.add_argument('--with-json-c', help='build json-c (default, version: git master)', dest='with_jsonc',
                           action='store_true', default=True)
    jsonc_grp.add_argument('--without-json-c', help='build without json-c', dest='with_jsonc', action='store_false',
                           default=False)

    # openssl
    openssl_grp = parser.add_mutually_exclusive_group()
    openssl_grp.add_argument('--with-openssl',
                             help='build openssl (default, version:{0})'.format(openssl_default_version),
                             dest='with_openssl', action='store_true', default=True)
    openssl_grp.add_argument('--without-openssl', help='build without openssl', dest='with_openssl',
                             action='store_false',
                             default=False)
    parser.add_argument('--openssl-version', help='openssl version (default: {0})'.format(openssl_default_version),
                        default=openssl_default_version)

    # libssh2
    libssh2_grp = parser.add_mutually_exclusive_group()
    libssh2_grp.add_argument('--with-libssh2', help='build libssh2 (default, version: git master)', dest='with_libssh2',
                             action='store_true', default=True)
    libssh2_grp.add_argument('--without-libssh2', help='build without libssh2', dest='with_libssh2',
                             action='store_false',
                             default=False)

    # qscintilla
    qscintilla_grp = parser.add_mutually_exclusive_group()
    qscintilla_grp.add_argument('--with-qscintilla', help='build qscintilla (default, version: git master)',
                                dest='with_qscintilla',
                                action='store_true', default=True)
    qscintilla_grp.add_argument('--without-qscintilla', help='build without qscintilla', dest='with_qscintilla',
                                action='store_false',
                                default=False)
    # common
    common_grp = parser.add_mutually_exclusive_group()
    common_grp.add_argument('--with-common', help='build common (default, version: git master)', dest='with_common',
                            action='store_true', default=True)
    common_grp.add_argument('--without-common', help='build without common', dest='with_common', action='store_false',
                            default=False)

    # hiredis
    hiredis_grp = parser.add_mutually_exclusive_group()
    hiredis_grp.add_argument('--with-hiredis', help='build hiredis (default, version: git master)',
                             dest='with_hiredis',
                             action='store_true', default=True)
    hiredis_grp.add_argument('--without-hiredis', help='build without hiredis', dest='with_hiredis',
                             action='store_false',
                             default=False)

    # libmemcached
    libmemcached_grp = parser.add_mutually_exclusive_group()
    libmemcached_grp.add_argument('--with-libmemcached', help='build libmemcached (default, version: git master)',
                                  dest='with_libmemcached',
                                  action='store_true', default=True)
    libmemcached_grp.add_argument('--without-libmemcached', help='build without libmemcached', dest='with_libmemcached',
                                  action='store_false',
                                  default=False)

    # unqlite
    unqlite_grp = parser.add_mutually_exclusive_group()
    unqlite_grp.add_argument('--with-unqlite', help='build unqlite (default, version: git master)',
                             dest='with_unqlite',
                             action='store_true', default=True)
    unqlite_grp.add_argument('--without-unqlite', help='build without unqlite', dest='with_unqlite',
                             action='store_false',
                             default=False)

    # lmdb
    lmdb_grp = parser.add_mutually_exclusive_group()
    lmdb_grp.add_argument('--with-lmdb', help='build lmdb (default, version: git master)',
                          dest='with_lmdb',
                          action='store_true', default=True)
    lmdb_grp.add_argument('--without-lmdb', help='build without lmdb', dest='with_lmdb',
                          action='store_false',
                          default=False)

    # leveldb
    leveldb_grp = parser.add_mutually_exclusive_group()
    leveldb_grp.add_argument('--with-leveldb', help='build leveldb (default, version: git master)',
                             dest='with_leveldb',
                             action='store_true', default=True)
    leveldb_grp.add_argument('--without-leveldb', help='build without leveldb', dest='with_leveldb',
                             action='store_false',
                             default=False)

    # rocksdb
    rocksdb_grp = parser.add_mutually_exclusive_group()
    rocksdb_grp.add_argument('--with-rocksdb', help='build rocksdb (default, version: git master)',
                             dest='with_rocksdb',
                             action='store_true', default=True)
    rocksdb_grp.add_argument('--without-rocksdb', help='build without rocksdb', dest='with_rocksdb',
                             action='store_false',
                             default=False)

    # forestdb
    forestdb_grp = parser.add_mutually_exclusive_group()
    forestdb_grp.add_argument('--with-forestdb', help='build forestdb (default, version: git master)',
                              dest='with_forestdb',
                              action='store_true', default=True)
    forestdb_grp.add_argument('--without-forestdb', help='build without forestdb', dest='with_forestdb',
                              action='store_false',
                              default=False)
    # fastonosql_core
    fastonosql_core_grp = parser.add_mutually_exclusive_group()
    fastonosql_core_grp.add_argument('--with-fastonosql-core',
                                     help='build fastonosql_core (default, version: git master)',
                                     dest='with_fastonosql_core',
                                     action='store_true', default=True)
    fastonosql_core_grp.add_argument('--without-fastonosql-core', help='build without fastonosql_core',
                                     dest='with_fastonosql_core',
                                     action='store_false',
                                     default=False)

    parser.add_argument('--platform', help='build for platform (default: {0})'.format(host_os), default=host_os)
    parser.add_argument('--architecture', help='architecture (default: {0})'.format(arch_host_os),
                        default=arch_host_os)
    parser.add_argument('--prefix_path', help='prefix_path (default: None)', default=None)

    argv = parser.parse_args()

    arg_platform = argv.platform
    arg_prefix_path = argv.prefix_path
    arg_architecture = argv.architecture

    request = BuildRequest(arg_platform, arg_architecture, 'build_' + arg_platform + '_env', arg_prefix_path)

    if argv.with_system:
        request.build_system()

    if argv.with_snappy:
        request.build_snappy()
    if argv.with_jsonc:
        request.build_jsonc()
    if argv.with_openssl:
        request.build_openssl(openssl_default_version)  #
    if argv.with_libssh2:
        request.build_libssh2()
    if argv.with_qscintilla:
        request.build_qscintilla()
    if argv.with_common:
        request.build_common(True)

    # databases libs builds
    if argv.with_hiredis:
        request.build_hiredis()
    if argv.with_libmemcached:
        request.build_libmemcached()  #
    if argv.with_unqlite:
        request.build_unqlite()
    if argv.with_lmdb:
        request.build_lmdb()
    if argv.with_leveldb:
        request.build_leveldb()
    if argv.with_rocksdb:
        request.build_rocksdb()
    if argv.with_forestdb:
        request.build_forestdb()  #

    if argv.with_fastonosql_core:
        request.build_fastonosql_core()
