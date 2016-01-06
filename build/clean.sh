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
    elif [ "$unamestr" = 'FreeBSD' ]; then
        platform='freebsd'
    fi 
fi

echo ========= START CLEANING ===========

echo platform: $platform
echo host: $unamestr

if [ "$platform" = 'windows' ]; then
    deleteDir build_windows
elif [ "$platform" = 'linux' ]; then
    deleteDir build_linux
elif [ "$platform"='macosx' ]; then
    deleteDir build_macosx
elif [ "$platform" = 'freebsd' ]; then
    deleteDir build_freebsd
elif [ "$platform" = 'android' ]; then
    deleteDir build_android
fi
echo ========= END CLEANING ===========
