#!/usr/bin/env python

import platform
import re

class Platform(object):
    def __init__(self, platform, archs, package_types):
        self.platform = platform
        self.archs = archs
        self.package_types = package_types

SUPPORTED_PLATFORMS = [Platform('linux', [32, 64], ['DEB', 'RPM', 'TGZ']),
                       Platform('windows', [32, 64], ['NSIS', 'ZIP']),
                       Platform('macosx', [64], ['DragNDrop', 'ZIP']),
                       Platform('freebsd', [64], ['TGZ']) ]

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
    else:
        return None

def get_os():
    uname_str = platform.system()
    if uname_str == 'MINGW64_NT-6.1':
        return 'windows'
    elif uname_str == 'Linux':
        return 'linux'
    elif uname_str == 'Darwin':
        return 'macosx'
    elif uname_str == 'FreeBSD':
        return 'freebsd'
    else:
        return None

def get_arch():
    arch = platform.architecture()
    return re.search(r'\d+', arch[0]).group()

def get_supported_platform_by_name(platform):
    return next((x for x in SUPPORTED_PLATFORMS if x.platform == platform), None)
    
def gen_routing_key(platform, arch):
    return platform + '_' + arch