LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := ModemInterface.c
# @TODO: add -Wall. This flag is missing because
# it seems to hide all the warnings.
LOCAL_CFLAGS := -Werror
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE := libamtl_jni
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

