# Copyright 2006-2014 The Android Open Source Project
# Copyright 2014      Intel

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := logcatext
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := intel

LOCAL_SRC_FILES:= logcat.cpp klogger.cpp unblocker.cpp

LOCAL_SHARED_LIBRARIES := liblog

LOCAL_CFLAGS := -Werror

ifeq ($(LOGCATEXT_USES_SVENTX),true)
LOCAL_CFLAGS += -DUSES_SVENTX
LOCAL_SHARED_LIBRARIES += libsventx
endif

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

#include $(call first-makefiles-under,$(LOCAL_PATH))
