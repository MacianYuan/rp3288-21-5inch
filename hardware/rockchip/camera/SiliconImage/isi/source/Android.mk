#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

#$(info my-dir=  $(call my-dir) )
#include $(all-subdir-makefiles)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
	isi.c\
	isisup.c\
	

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../include\
	$(LOCAL_PATH)/../include_priv\
	$(LOCAL_PATH)/../../include\


LOCAL_CFLAGS := -Wall -Wextra -std=c99   -Wformat-nonliteral -g -O0 -DDEBUG -pedantic
LOCAL_CFLAGS += -DLINUX  -DMIPI_USE_CAMERIC -DHAL_MOCKUP -DCAM_ENGINE_DRAW_DOM_ONLY -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3326)
LOCAL_SHARED_LIBRARIES += \
        lib_rkisp12_api
else
LOCAL_SHARED_LIBRARIES += \
        lib_rkisp1_api
endif

LOCAL_MODULE:= libisp_isi

LOCAL_MODULE_TAGS:= optional
#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

