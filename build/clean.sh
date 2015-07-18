#!/bin/sh
set -e

deleteDir() {
dir_path="$1"
if [ -d "$dir_path" ]; then
    echo delete $dir_path
      rm -rf "$dir_path"
    fi
}

unamestr=`uname`
if [ -n "$1" ]; then
     platform=$1
else
    if [ "$unamestr" = 'MINGW64_NT-6.1' ]; then
        platform='windows'
    elif [ "$unamestr" = 'Linux' ]; then
        platform='linux'
    elif [ "$unamestr" = 'Darwin' ]; then
        platform='macosx'
    fi 
fi

echo ========= START CLEANING ===========

echo platform: $platform
echo host: $unamestr

if [ "$platform" = 'windows' ]; then
    deleteDir build_nsis
    deleteDir build_zip
elif [ "$platform" = 'linux' ]; then
    deleteDir build_deb
    deleteDir build_rpm
    deleteDir build_tar
elif [ "$platform"='macosx' ]; then
    deleteDir build_dmg
    deleteDir build_zip
elif [ "$platform" = 'android' ]; then
    deleteDir build_apk
fi
echo ========= END CLEANING ===========
