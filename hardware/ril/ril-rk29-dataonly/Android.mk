# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
#
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril-rk29-dataonly.c \
    atchannel.c \
    misc.c \
    at_tok.c \
    extend_at_func.c \
    modem_define_func.c \
    operator_table.c
#gsm.c
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := \
    liblog libcutils libutils libril libhardware_legacy libnetutils librilutils

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_CFLAGS += -Wno-unused-parameter

LOCAL_C_INCLUDES :=

ifeq ($(TARGET_DEVICE),sooner)
  LOCAL_CFLAGS += -DOMAP_CSMI_POWER_CONTROL -DUSE_TI_COMMANDS
endif

ifeq ($(TARGET_DEVICE),surf)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

ifeq ($(TARGET_DEVICE),dream)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

LOCAL_PRELINK_MODULE := false

ifeq (foo,foo)
  #build shared library
  LOCAL_SHARED_LIBRARIES += \
      libcutils libutils
#  LOCAL_LDLIBS += -lpthread
  LOCAL_CFLAGS += -DRIL_SHLIB
  LOCAL_PROPRIETARY_MODULE := true
  LOCAL_MODULE:= libril-rk29-dataonly
  include $(BUILD_SHARED_LIBRARY)
else
  #build executable
  LOCAL_SHARED_LIBRARIES += \
      libril
  LOCAL_MODULE:= reference-ril
  include $(BUILD_EXECUTABLE)
endif
