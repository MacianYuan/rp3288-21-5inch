LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LIBUSB_DIR :=external/libusb
common_src := libusb/core.c
common_include :=	$(LOCAL_PATH)/ \
			$(LOCAL_PATH)/libusb \
			$(LIBUSB_DIR)/libusb
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := libusb-compat
LOCAL_SRC_FILES :=$(common_src)
LOCAL_C_INCLUDES +=$(common_include)
LOCAL_SHARED_LIBRARIES := libusb
include $(BUILD_SHARED_LIBRARY)
