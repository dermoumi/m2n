LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

BASE_PATH := ${LOCAL_PATH}/../../../../../..
SRC_PATH := ${BASE_PATH}/src

LOCAL_MODULE := main

LOCAL_C_INCLUDES = \
    ${SRC_PATH} \
    ${BASE_PATH}/extlibs/include

LOCAL_SRC_FILES := $(subst ${LOCAL_PATH}/,, \
    $(wildcard ${SRC_PATH}/*.cpp) \
    $(wildcard ${SRC_PATH}/*/*.cpp))

LOCAL_CFLAGS := -std=c++11 -DNX_OPENGL_ES -DGL_GLEXT_PROTOTYPES
LOCAL_LDFLAGS := ${LOCAL_LDFLAGS} -export-dynamic
LOCAL_STATIC_LIBRARIES := SDL2_static SoLoud freetype2-static PhysFS libluajit
LOCAL_LDLIBS := -ldl -llog -lGLESv2

$(call import-add-path,${BASE_PATH}/extlibs/android)

include $(BUILD_SHARED_LIBRARY)

$(call import-module,SDL2)LOCAL_PATH := $(call my-dir)
