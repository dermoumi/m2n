LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

BASE_PATH := ${LOCAL_PATH}/../../../../../..
SRC_PATH := ${BASE_PATH}/extlibs/src/freetype

LOCAL_MODULE := freetype2-static

LOCAL_CFLAGS := -DANDROID_NDK -DFT2_BUILD_LIBRARY=1

LOCAL_C_INCLUDES := \
    ${SRC_PATH}

LOCAL_SRC_FILES := $(subst ${LOCAL_PATH}/,, \
    $(SRC_PATH)/autofit/autofit.c \
	$(SRC_PATH)/base/basepic.c \
	$(SRC_PATH)/base/ftapi.c \
	$(SRC_PATH)/base/ftbase.c \
	$(SRC_PATH)/base/ftbbox.c \
	$(SRC_PATH)/base/ftbitmap.c \
	$(SRC_PATH)/base/ftdbgmem.c \
	$(SRC_PATH)/base/ftdebug.c \
	$(SRC_PATH)/base/ftglyph.c \
	$(SRC_PATH)/base/ftinit.c \
	$(SRC_PATH)/base/ftpic.c \
	$(SRC_PATH)/base/ftstroke.c \
	$(SRC_PATH)/base/ftsynth.c \
	$(SRC_PATH)/base/ftsystem.c \
	$(SRC_PATH)/cff/cff.c \
	$(SRC_PATH)/pshinter/pshinter.c \
	$(SRC_PATH)/psnames/psnames.c \
	$(SRC_PATH)/raster/raster.c \
	$(SRC_PATH)/sfnt/sfnt.c \
	$(SRC_PATH)/smooth/smooth.c \
	$(SRC_PATH)/truetype/truetype.c)

include $(BUILD_STATIC_LIBRARY)

