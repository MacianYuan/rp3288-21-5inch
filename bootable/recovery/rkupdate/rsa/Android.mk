LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := librkrsa
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
ifneq ($(strip $(TARGET_2ND_ARCH)), )
  LOCAL_MULTILIB := both
  LOCAL_SRC_FILES_$(TARGET_ARCH) := lib64/librkrsa.a
  LOCAL_SRC_FILES_$(TARGET_2ND_ARCH) := lib/librkrsa.a
else
  LOCAL_SRC_FILES := lib/librkrsa.a
endif

include $(BUILD_PREBUILT)
