LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PROPRIETARY_MODULE := true
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
	ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3326)
		LOCAL_MODULE := lib_rkisp12_api
		LOCAL_SRC_FILES_arm := $(LOCAL_MODULE)_32bit.so
		LOCAL_SRC_FILES_arm64 := $(LOCAL_MODULE)_64bit.so
	else
		LOCAL_MODULE := lib_rkisp1_api
		LOCAL_SRC_FILES_arm := $(LOCAL_MODULE)_32bit.so
		LOCAL_SRC_FILES_arm64 := $(LOCAL_MODULE)_64bit.so
	endif
endif

#include $(CLEAR_VARS)
ifneq ($(strip $(TARGET_2ND_ARCH)), )
	LOCAL_MULTILIB := both
endif
LOCAL_MODULE_RELATIVE_PATH :=

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_STEM := $(LOCAL_MODULE)
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)

