#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
	source/SP2519_PARREL.c\
	source/SP2519_tables.c\
	

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include_priv\
	$(LOCAL_PATH)/../../include\
	$(LOCAL_PATH)/../../include_priv\
	$(LOCAL_PATH)/../../../include\
	
LOCAL_LDLIBS := -llog	

LOCAL_CFLAGS := -Wall -Wextra -std=c99   -Wformat-nonliteral -g -O0 -DDEBUG -pedantic -Wno-zero-length-array -Wno-unused-parameter -Wno-unused-variable -Wno-c11-extensions -Wno-unused-function \
-Wno-zero-length-array -Wno-unused-parameter -Wno-unused-variable -Wno-c11-extensions -Wno-unused-function -Wno-missing-field-initializers -Wno-sometimes-uninitialized -Wno-parentheses-equality

LOCAL_CFLAGS += -DLINUX  -DMIPI_USE_CAMERIC -DHAL_MOCKUP -DCAM_ENGINE_DRAW_DOM_ONLY -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H
#LOCAL_STATIC_LIBRARIES := libisp_ebase libisp_oslayer libisp_common libisp_hal libisp_cameric_reg_drv libisp_cameric_drv libisp_isi
LOCAL_SHARED_LIBRARIES := libutils libcutils libion
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3326)
LOCAL_SHARED_LIBRARIES += \
	lib_rkisp12_api
else
LOCAL_SHARED_LIBRARIES += \
	lib_rkisp1_api
endif
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
	LOCAL_PROPRIETARY_MODULE := true
endif
LOCAL_MODULE:= libisp_isi_drv_SP2519

#LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
ifneq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 5.0)))
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
else
ifneq ($(strip $(TARGET_2ND_ARCH)), )
LOCAL_MULTILIB := both
endif
LOCAL_MODULE_RELATIVE_PATH := hw
endif

LOCAL_MODULE_TAGS:= optional
include $(BUILD_SHARED_LIBRARY)

