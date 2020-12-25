################################################

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    camera_test.c \
    OV8858_MIPI.c \
    GC2155_MIPI.c
	
LOCAL_C_INCLUDES += \
	frameworks/base/include/ui \
  bionic\
  external/stlport/stlport\

LOCAL_SHARED_LIBRARIES := \
    libui \
    libbinder \
    libutils \
    libcutils \
    libion \
    libdl

LOCAL_CFLAGS += -fno-short-enums

LOCAL_MODULE:= libcamsys

include $(BUILD_STATIC_LIBRARY)

