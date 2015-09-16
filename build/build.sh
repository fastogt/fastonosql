#!/bin/sh
set -e

createPackage() {
    branding_variables="$(cat $1)"
    branding_complex_variables="$(cat $2)"
	#echo branding_variables: $branding_variables
	#echo branding_complex_variables: $branding_complex_variables 
    platform="$3"
	os_arch="$4"
    dir_path="$5"
    cpack_generator="$6"
	
    if [ -d "$dir_path" ]; then
        rm -rf "$dir_path"
    fi
    mkdir "$dir_path"
    cd "$dir_path"
    if [ "$platform" = 'android' ] ; then
        if [ -n "$branding_complex_variables" ]; then
            cmake ../../ -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=RELEASE -DOS_ARCH=$os_arch \
            -DOPENSSL_USE_STATIC=1 $branding_variables "$branding_complex_variables"
        else
            cmake ../../ -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake -DCMAKE_BUILD_TYPE=RELEASE -DOS_ARCH=$os_arch \
            -DOPENSSL_USE_STATIC=1 $branding_variables
        fi
        make install -j2
        make apk_release
        make apk_signed
		make apk_signed_aligned
    else
        if [ -n "$branding_complex_variables" ]; then
            cmake ../../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RELEASE -DOS_ARCH=$os_arch -DOPENSSL_USE_STATIC=1 -DCPACK_GENERATOR="$cpack_generator" \
            $branding_variables "$branding_complex_variables"
        else
            cmake ../../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RELEASE -DOS_ARCH=$os_arch -DOPENSSL_USE_STATIC=1 -DCPACK_GENERATOR="$cpack_generator" \
            $branding_variables 
        fi
        make install -j2
		cpack -G "$cpack_generator"
    fi
    
#    if [ "$cpack_generator" = 'DEB' ]; then
#        sh ./fixup_deb.sh
#    fi
    cd ../
}

#args: 
#1 - branding_file
#2 - branding_complex_file
#3 - platform (windows, linux, macosx, freebsd, android)
#4 - os_arch (32, 64)

unamestr=`uname`
unamearch=`uname -m`

if [ -n "$1" ]; then
    branding_file=$1
else
echo "Please specify branding file for simple variables or /dev/null!"
	exit 0
fi

if [ -n "$2" ]; then
    branding_complex_file=$2
else
echo "Please specify branding file for complex variables or /dev/null!"
	exit 0
fi

if [ -n "$3" ]; then
    platform=$3
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

if [ -n "$4" ]; then
	if [ "$4" != 32 ]; then 
		if [ "$4" != 64 ]; then
			echo "Please specify valid os architecture 32 or 64"
			exit 0
		fi
	fi
    os_arch=$4
else
    os_arch=64
fi

echo ========= START BUILDING ===========
echo platform: $platform
echo arch: $os_arch
echo branding file: $branding_file
echo branding complex file: $branding_complex_file
echo host: $unamestr
echo host_arch: $unamearch

#-DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/

if [ "$platform" = 'windows' ]; then
	echo Build for Windows ...
    createPackage $branding_file $branding_complex_file $platform $os_arch build_nsis NSIS
    createPackage $branding_file $branding_complex_file $platform $os_arch build_zip ZIP
elif [ "$platform" = 'linux' ]; then
	echo Build for Linux ...
    createPackage $branding_file $branding_complex_file $platform $os_arch build_deb DEB
    createPackage $branding_file $branding_complex_file $platform $os_arch build_rpm RPM
    createPackage $branding_file $branding_complex_file $platform $os_arch build_tar TGZ
elif [ "$platform" = 'macosx' ]; then
	echo Build for MacOSX ...
    createPackage $branding_file $branding_complex_file $platform $os_arch build_dmg DragNDrop
    createPackage $branding_file $branding_complex_file $platform $os_arch build_zip ZIP
elif [ "$platform" = 'freebsd' ]; then
    echo Build for FreeBSD ...
    createPackage $branding_file $branding_complex_file $platform $os_arch build_tar TGZ
elif [ "$platform" = 'android' ]; then
    echo Build for Android ...
    createPackage $branding_file $branding_complex_file $platform 32 build_apk APK
fi

echo ========= END BUILDING ===========
