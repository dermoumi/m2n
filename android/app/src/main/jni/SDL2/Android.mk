LOCAL_PATH := $(call my-dir)

###########################
#
# SDL shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2


BASE_PATH := ${LOCAL_PATH}/../../../../../..
SRC_PATH := ${BASE_PATH}/extlibs/src/SDL2

LOCAL_C_INCLUDES = \
	${SRC_PATH} \
	${SRC_PATH}/include 

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(SRC_PATH)/src/*.c) \
	$(wildcard $(SRC_PATH)/src/audio/*.c) \
	$(wildcard $(SRC_PATH)/src/audio/android/*.c) \
	$(wildcard $(SRC_PATH)/src/audio/dummy/*.c) \
	$(SRC_PATH)/src/atomic/SDL_atomic.c \
	$(SRC_PATH)/src/atomic/SDL_spinlock.c.arm \
	$(wildcard $(SRC_PATH)/src/core/android/*.c) \
	$(wildcard $(SRC_PATH)/src/cpuinfo/*.c) \
	$(wildcard $(SRC_PATH)/src/dynapi/*.c) \
	$(wildcard $(SRC_PATH)/src/events/*.c) \
	$(wildcard $(SRC_PATH)/src/file/*.c) \
	$(wildcard $(SRC_PATH)/src/haptic/*.c) \
	$(wildcard $(SRC_PATH)/src/haptic/dummy/*.c) \
	$(wildcard $(SRC_PATH)/src/joystick/*.c) \
	$(wildcard $(SRC_PATH)/src/joystick/android/*.c) \
	$(wildcard $(SRC_PATH)/src/loadso/dlopen/*.c) \
	$(wildcard $(SRC_PATH)/src/power/*.c) \
	$(wildcard $(SRC_PATH)/src/power/android/*.c) \
	$(wildcard $(SRC_PATH)/src/filesystem/android/*.c) \
	$(wildcard $(SRC_PATH)/src/render/*.c) \
	$(wildcard $(SRC_PATH)/src/render/*/*.c) \
	$(wildcard $(SRC_PATH)/src/stdlib/*.c) \
	$(wildcard $(SRC_PATH)/src/thread/*.c) \
	$(wildcard $(SRC_PATH)/src/thread/pthread/*.c) \
	$(wildcard $(SRC_PATH)/src/timer/*.c) \
	$(wildcard $(SRC_PATH)/src/timer/unix/*.c) \
	$(wildcard $(SRC_PATH)/src/video/*.c) \
	$(wildcard $(SRC_PATH)/src/video/android/*.c) \
	$(wildcard $(SRC_PATH)/src/test/*.c))

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid

# include $(BUILD_SHARED_LIBRARY)

###########################
#
# SDL static library
#
###########################

LOCAL_MODULE := SDL2_static

LOCAL_MODULE_FILENAME := libSDL2

LOCAL_SRC_FILES += $(subst $(LOCAL_PATH)/,,$(SRC_PATH)/src/main/android/SDL_android_main.c)

LOCAL_LDLIBS := 
LOCAL_EXPORT_LDLIBS := -Wl,--undefined=Java_org_libsdl_app_SDLActivity_nativeInit -ldl -lGLESv1_CM -lGLESv2 -llog -landroid

include $(BUILD_STATIC_LIBRARY)
