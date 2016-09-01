#!/usr/bin/env python

import platform
import re

class Architecture(object):
    def __init__(self, arch_str, bit):
        self.name = arch_str
        self.bit = bit

class Platform(object):
    def __init__(self, name, archs, package_types):
        self.name = name
        self.archs = archs
        self.package_types = package_types

    def architecture_by_arch(self, arch):
        for curr_arch in self.archs:
            if (curr_arch.name == arch):
                return curr_arch

        return None

    def architecture_by_bit(self, arch_bit):
        for curr_arch in self.archs:
            if (curr_arch.bit == arch_bit):
                return curr_arch

        return None

SUPPORTED_PLATFORMS = [Platform('linux', [Architecture('x86_64', 64), Architecture('i386', 32)], ['DEB', 'RPM', 'TGZ']),
                       Platform('windows', [Architecture('x86_64', 64), Architecture('i386', 32)], ['NSIS', 'ZIP']),
                       Platform('macosx', [Architecture('x86_64', 64)], ['DragNDrop', 'ZIP']),
                       Platform('freebsd', [Architecture('x86_64', 64)], ['TGZ']),
                       Platform('android', [Architecture('armv7', 32)], ['APK'])]

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
    if uname_str == 'MINGW64_NT-6.1':
        return 'windows'
    if uname_str == 'Windows':
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
    return next((x for x in SUPPORTED_PLATFORMS if x.name == platform), None)
    
def gen_routing_key(platform, arch):
    return platform + '_' + arch