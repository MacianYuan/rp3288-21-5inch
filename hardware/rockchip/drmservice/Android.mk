LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
#LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_SRC_FILES:= \
    drmservice.c

LOCAL_MULTILIB := 32

LOCAL_C_INCLUDES += bionic \
$(call include-path-for, libhardware_legacy)/hardware_legacy

LOCAL_MODULE:=drmservice
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := drmservice.rc
LOCAL_STATIC_LIBRARIES := libfs_mgr libcutils 
#libc

LOCAL_SHARED_LIBRARIES := libhardware_legacy libnetutils liblog

include $(BUILD_EXECUTABLE)
