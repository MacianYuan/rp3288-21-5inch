LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

INCLUDES = $(LOCAL_PATH)
INCLUDES += external/libusb/libusb
INCLUDES += external/libusb-compat/libusb

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := usb_modeswitch
LOCAL_SRC_FILES := usb_modeswitch.c
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES :=$(INCLUDES)
LOCAL_SHARED_LIBRARIES := libusb libusb-compat

$(shell mkdir -p $(PRODUCT_OUT)/vendor/etc)
$(shell cp -R $(LOCAL_PATH)/usb_modeswitch.d $(PRODUCT_OUT)/vendor/etc)

include $(BUILD_EXECUTABLE)

