#!/bin/sh
set -e

createPackage() {
    platform="$1"
    dir_path="$2"
    cpack_generator="$3"

    if [ -d "$dir_path" ]; then
        rm -rf "$dir_path"
    fi
    mkdir "$dir_path"
    cd "$dir_path"
    if [ "$platform" = 'android' ] ; then
        cmake ../../ -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=RELEASE -DOS_ARCH=32 -DOPENSSL_USE_STATIC=1
        make install -j2
        make apk_release
        make apk_signed
	make apk_signed_aligned
    else
        cmake ../../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RELEASE -DOS_ARCH=64 -DOPENSSL_USE_STATIC=1 -DCPACK_GENERATOR="$cpack_generator"
        make install -j2
		cpack -G "$cpack_generator"
    fi
    
#    if [ "$cpack_generator" = 'DEB' ]; then
#        sh ./fixup_deb.sh
#    fi
    cd ../
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

echo ========= START BUILDING ===========
echo platform: $platform
echo host: $unamestr

#-DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/

if [ "$platform" = 'windows' ]; then
	echo Build for Windows ...
    createPackage $platform build_nsis NSIS
    createPackage $platform build_zip ZIP
elif [ "$platform" = 'linux' ]; then
	echo Build for Linux ...
    createPackage $platform build_deb DEB
    createPackage $platform build_rpm RPM
    createPackage $platform build_tar TGZ
elif [ "$platform" = 'macosx' ]; then
	echo Build for MacOSX ...
    createPackage $platform build_dmg DragNDrop  
    createPackage $platform build_zip ZIP 
elif [ "$platform" = 'android' ]; then
    echo Build for Android ...
    createPackage $platform build_apk APK
fi

echo ========= END BUILDING ===========
