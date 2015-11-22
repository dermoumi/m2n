#!/bin/bash
#================================================================#
# Build script based on 
#   https://github.com/moai/moai-dev/blob/master/ant/libmoai/jni/luajit/build.sh
#================================================================#

luajitsrc=../../../../../../extlibs/src/LuaJIT/
luajitdst=${PWD}

host_os=`uname -s | tr "[:upper:]" "[:lower:]"`
host_arch=`uname -m`

if [ -f android/armeabi/libluajit.a ] &&
    [ -f android/armeabi-v7a/libluajit.a ]; then
    echo "LuaJIT Already built"
    exit 0
fi

NDK_BUILD_LOCATION=${ANDROID_NDK}
NDK="${NDK_BUILD_LOCATION%/ndk-build}"
if [[ x$NDK = "x" ]]; then
    echo "The Android NDK (ANDROID_NDK) must be on your path."
    exit 1
fi

copyAndClean()
{
    arch="$1"
    DESTDIR=$luajitdst/android/$arch
    mkdir -p $DESTDIR 2>/dev/null
    rm "$DESTDIR"/*.a 2>/dev/null

    if [ -f libluajit.a ]; then
        mv libluajit.a $DESTDIR/libluajit.a
    fi;
    
    make clean
}

CFLAGS=""

cd $luajitsrc/src

# Android/ARM, armeabi (ARMv5TE soft-float), Android 2.2+ (Froyo)
NDKABI=8
NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
NDKP=$NDKVER/prebuilt/${host_os}-${host_arch}/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
make HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF" TARGET_CFLAGS="$CFLAGS" libluajit.a
copyAndClean armeabi

# Android/ARM, armeabi-v7a (ARMv7 VFP), Android 4.0+ (ICS)
NDKABI=14
NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
NDKP=$NDKVER/prebuilt/${host_os}-${host_arch}/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
NDKARCH="-march=armv7-a -mfloat-abi=softfp -Wl,--fix-cortex-a8"
make HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_CFLAGS="$CFLAGS" libluajit.a
copyAndClean armeabi-v7a

# Android/MIPS, mips (MIPS32R1 hard-float), Android 4.0+ (ICS)
NDKABI=14
NDKVER=$NDK/toolchains/mipsel-linux-android-4.9
NDKP=$NDKVER/prebuilt/${host_os}-${host_arch}/bin/mipsel-linux-android-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-mips"
make HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF" TARGET_CFLAGS="$CFLAGS" libluajit.a
copyAndClean mips

# Android/x86, x86 (i686 SSE3), Android 4.0+ (ICS)
NDKABI=14
NDKVER=$NDK/toolchains/x86-4.9
NDKP=$NDKVER/prebuilt/${host_os}-${host_arch}/bin/i686-linux-android-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-x86"
make HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF" TARGET_CFLAGS="$CFLAGS" libluajit.a
copyAndClean x86
