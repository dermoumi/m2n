LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)


BASE_PATH := ${LOCAL_PATH}/../../../../../..
SRC_PATH := ${BASE_PATH}/extlibs/src/PhysFS

LOCAL_MODULE    := PhysFS


LOCAL_CPPFLAGS  := ${LOCAL_CFLAGS}

LOCAL_C_INCLUDES  :=  \
    ${LOCAL_PATH}/src
        
LOCAL_SRC_FILES := \
    $(filter-out \
  , $(subst $(LOCAL_PATH)/,,\
    ${SRC_PATH}/lzma/C/7zCrc.c \
    ${SRC_PATH}/lzma/C/Compress/Lzma/LzmaDecode.c \
    $(wildcard ${SRC_PATH}/*.c) \
    $(wildcard ${SRC_PATH}/lzma/C/Archive/7z/*.c) \
    $(wildcard ${SRC_PATH}/lzma/C/Compress/Branch/*.c)))

LOCAL_CFLAGS    := -fexceptions -g -Dlinux -Dunix \
    -DPHYSFS_NO_CDROM_SUPPORT=1 \
    -DHAVE_GCC_DESTRUCTOR=1 -DOPT_GENERIC -DREAL_IS_FLOAT \
    -DPHYSFS_SUPPORTS_ZIP=1 -DPHYSFS_SUPPORTS_7Z=1 \
    -DPHYSFS_SUPPORTS_GRP=0 -DPHYSFS_SUPPORTS_QPAK=0 \
    -DPHYSFS_SUPPORTS_HOG=0 -DPHYSFS_SUPPORTS_MVL=0 \
    -DPHYSFS_SUPPORTS_WAD=0

include $(BUILD_STATIC_LIBRARY)