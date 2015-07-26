#!/bin/sh
set -e

createPackage() {
	branding_variables=$(<$1)
    platform="$2"
    dir_path="$3"
    cpack_generator="$4"
	
    if [ -d "$dir_path" ]; then
        rm -rf "$dir_path"
    fi
    mkdir "$dir_path"
    cd "$dir_path"
    if [ "$platform" = 'android' ] ; then
        cmake ../../ -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=RELEASE -DOS_ARCH=32 -DOPENSSL_USE_STATIC=1 $branding_variables
        make install -j2
        make apk_release
        make apk_signed
		make apk_signed_aligned
    else
        cmake ../../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RELEASE -DOS_ARCH=64 -DOPENSSL_USE_STATIC=1 -DCPACK_GENERATOR="$cpack_generator" $branding_variables
        make install -j2
		cpack -G "$cpack_generator"
    fi
    
#    if [ "$cpack_generator" = 'DEB' ]; then
#        sh ./fixup_deb.sh
#    fi
    cd ../
}

if [ "$#" -ne 1 ]; then
    echo "Please specify at least branding file or /dev/null!"
	exit 0
fi

unamestr=`uname`
if [ -n "$1" ]; then
    branding_file=$1
fi

if [ -n "$2" ]; then
    platform=$2
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

echo ========= START BUILDING ===========
echo platform: $platform
echo branding file: $branding_file
echo host: $unamestr

#-DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/

if [ "$platform" = 'windows' ]; then
	echo Build for Windows ...
    createPackage $branding_file $platform build_nsis NSIS
    createPackage $branding_file $platform build_zip ZIP
elif [ "$platform" = 'linux' ]; then
	echo Build for Linux ...
    createPackage $branding_file $platform build_deb DEB
    createPackage $branding_file $platform build_rpm RPM
    createPackage $branding_file $platform build_tar TGZ
elif [ "$platform" = 'macosx' ]; then
	echo Build for MacOSX ...
    createPackage $branding_file $platform build_dmg DragNDrop  
    createPackage $branding_file $platform build_zip ZIP
elif [ "$platform" = 'freebsd' ]; then
    echo Build for FreeBSD ...
    createPackage $branding_file $platform build_tar TGZ
elif [ "$platform" = 'android' ]; then
    echo Build for Android ...
    createPackage $branding_file $platform build_apk APK
fi

echo ========= END BUILDING ===========
