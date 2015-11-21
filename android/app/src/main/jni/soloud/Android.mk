LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

BASE_PATH := ${LOCAL_PATH}/../../../../../..
SRC_PATH := ${BASE_PATH}/extlibs/src/soloud

LOCAL_MODULE := SoLoud
#LOCAL_MODULE_FILENAME := libSoloud

LOCAL_C_INCLUDES = \
    ${SRC_PATH} \
    ${BASE_PATH}/extlibs/include \
    ${BASE_PATH}/extlibs/include/soloud

LOCAL_SRC_FILES := $(subst ${LOCAL_PATH}/,, \
    $(wildcard ${SRC_PATH}/*/*.c*) \
    $(wildcard ${SRC_PATH}/*/*/*.c*))

LOCAL_CFLAGS := -DWITH_SDL2_STATIC

include $(BUILD_STATIC_LIBRARY)
