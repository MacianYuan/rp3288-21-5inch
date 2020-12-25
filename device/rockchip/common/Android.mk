LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_ROOT_OUT)/metadata)
$(shell mkdir -p $(TARGET_ROOT_OUT)/cache)
include $(call all-makefiles-under,$(LOCAL_PATH))
