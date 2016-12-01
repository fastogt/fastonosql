#!/usr/bin/env python

import platform
import re

class Architecture(object):
    def __init__(self, arch_str, bit, default_install_prefix_path):
        self.name_ = arch_str
        self.bit_ = bit
        self.default_install_prefix_path_ = default_install_prefix_path

    def name(self):
        return self.name_

    def bit(self):
        return self.bit_

    def default_install_prefix_path(self):
        return self.default_install_prefix_path_

class Platform(object):
    def __init__(self, name, arch, package_types):
        self.name_ = name
        self.arch_ = arch
        self.package_types_ = package_types

    def name(self):
        return self.name_

    def arch(self):
        return self.arch_

    def package_types(self):
        return self.package_types_

class SupportedPlatform(object):
    def __init__(self, name, archs, package_types):
        self.name_ = name
        self.archs_ = archs
        self.package_types_ = package_types

    def name(self):
        return self.name_

    def archs(self):
        return self.archs_

    def package_types(self):
        return self.package_types_

    def architecture_by_arch(self, arch):
        for curr_arch in self.archs_:
            if (curr_arch.name == arch):
                return curr_arch

        return None

    def architecture_by_bit(self, arch_bit):
        for curr_arch in self.archs_:
            if (curr_arch.bit() == arch_bit):
                return curr_arch

        return None

SUPPORTED_PLATFORMS = [SupportedPlatform('linux', [Architecture('x86_64', 64, '/usr/local'), Architecture('i386', 32, '/usr/local')], ['DEB', 'RPM', 'TGZ']),
                       SupportedPlatform('windows', [Architecture('x86_64', 64, '/mingw64'), Architecture('i386', 32, '/mingw32')], ['NSIS', 'ZIP']),
                       SupportedPlatform('macosx', [Architecture('x86_64', 64, '/usr/local')], ['DragNDrop', 'ZIP']),
                       SupportedPlatform('freebsd', [Architecture('x86_64', 64, '/usr/local')], ['TGZ']),
                       SupportedPlatform('android', [Architecture('armv7', 32, '/opt/android-ndk/platforms/android-9/arch-arm/usr/')], ['APK'])]

def get_extension_by_package(package_type):
    if package_type == 'DEB':
        return 'deb'
    elif package_type == 'RPM':
        return 'rpm'
    elif package_type == 'TGZ':
        return 'tar.gz'
    elif package_type == 'NSIS':
        return 'exe'
    elif package_type == 'ZIP':
        return 'zip'
    elif package_type == 'DragNDrop':
        return 'dmg'
    elif package_type == 'APK':
        return 'apk'
    else:
        return None

def get_os():
    uname_str = platform.system()
    if 'MINGW' in uname_str:
        return 'windows'
    elif uname_str == 'Windows':
        return 'windows'
    elif uname_str == 'Linux':
        return 'linux'
    elif uname_str == 'Darwin':
        return 'macosx'
    elif uname_str == 'FreeBSD':
        return 'freebsd'
    elif uname_str == 'Android':
        return 'android'
    else:
        return None

def get_arch_bit():
    arch = platform.architecture()
    return re.search(r'\d+', arch[0]).group()

def get_supported_platform_by_name(platform):
    return next((x for x in SUPPORTED_PLATFORMS if x.name() == platform), None)
    
def gen_routing_key(platform, arch):
    return platform + '_' + arch

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
                           BuildSystem('make', ['make', '-j2'], '-GUnix Makefiles'),
                           BuildSystem('gmake', ['gmake', '-j2'], '-GUnix Makefiles')]

def get_supported_build_system_by_name(name):
    return next((x for x in SUPPORTED_BUILD_SYSTEMS if x.name() == name), None)
