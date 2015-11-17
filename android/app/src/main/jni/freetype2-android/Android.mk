LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libfreetype2-static
LOCAL_SRC_FILES := android/$(TARGET_ARCH_ABI)/libfreetype2-static.a

include $(PREBUILT_STATIC_LIBRARY)
