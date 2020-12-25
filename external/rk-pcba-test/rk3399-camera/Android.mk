################################################

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    camera_test.c \
	OV5648_MIPI.c \
	OV13850_MIPI.c

LOCAL_C_INCLUDES += \
    frameworks/base/include/ui \
    bionic \
    external/stlport/stlport

LOCAL_SHARED_LIBRARIES := \
    libui \
    libbinder \
    libutils \
    libcutils \
    libhardware \
	libion \
    libdl \
    libgralloc_drm

LOCAL_CFLAGS += -fno-short-enums

LOCAL_MODULE:= libcamsys

include $(BUILD_STATIC_LIBRARY)

