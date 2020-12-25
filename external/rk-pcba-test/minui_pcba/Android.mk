LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := graphics.c graphics_adf.c graphics_drm.c graphics_fbdev.c events.c \
	resources.c

LOCAL_C_INCLUDES +=\
    external/libpng\
    external/zlib\
    external/libdrm/include/drm\
    external/libdrm


LOCAL_WHOLE_STATIC_LIBRARIES += libadf

ifneq ($(filter $(strip $(TARGET_BOARD_PLATFORM)), rk3326 rk3399), )
LOCAL_WHOLE_SHARED_LIBRARIES += libdrm
else
LOCAL_WHOLE_STATIC_LIBRARIES += libdrm
endif
LOCAL_WHOLE_STATIC_LIBRARIES += libpixelflinger_twrp

LOCAL_MODULE := libminuitwrp

# This used to compare against values in double-quotes (which are just
# ordinary characters in this context).  Strip double-quotes from the
# value so that either will work.

ifeq ($(subst ",,$(TARGET_RECOVERY_ROTATION)),270)
  LOCAL_CFLAGS += -DRECOVERY_ROTATION_270
  $(warning recovery rotation 270)
endif

ifeq ($(subst ",,$(TARGET_RECOVERY_ROTATION)),180)
  LOCAL_CFLAGS += -DRECOVERY_ROTATION_180
  $(warning recovery rotation 180)
endif

ifeq ($(subst ",,$(TARGET_RECOVERY_ROTATION)),90)
  LOCAL_CFLAGS += -DRECOVERY_ROTATION_90
  $(warning recovery rotation 90)
endif



ifeq ($(subst ",,$(TARGET_RECOVERY_PIXEL_FORMAT)),RGBX_8888)
  LOCAL_CFLAGS += -DRECOVERY_RGBX
endif
ifeq ($(subst ",,$(TARGET_RECOVERY_PIXEL_FORMAT)),BGRA_8888)
  LOCAL_CFLAGS += -DRECOVERY_BGRA
endif



ifneq ($(TARGET_RECOVERY_OVERSCAN_PERCENT),)
  LOCAL_CFLAGS += -DOVERSCAN_PERCENT=$(TARGET_RECOVERY_OVERSCAN_PERCENT)
else
  LOCAL_CFLAGS += -DOVERSCAN_PERCENT=0
endif

ifeq ($(strip $(TARGET_ARCH)), arm64)
  LOCAL_CFLAGS += -DTARGET_ARCH_64
endif

include $(BUILD_STATIC_LIBRARY)

define _add-font-image
include $$(CLEAR_VARS)
LOCAL_MODULE := bootable_recovery_minui_$(notdir $(1))
LOCAL_MODULE_STEM := $(notdir $(1))
_img_modules += $$(LOCAL_MODULE)
LOCAL_SRC_FILES := $1
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $$(TARGET_ROOT_OUT)/res/images
include $$(BUILD_PREBUILT)
endef

_img_modules :=
_images :=
$(foreach _img, $(call find-subdir-subdir-files, "images", "*.png"), \
  $(eval $(call _add-font-image,$(_img))))

include $(CLEAR_VARS)
LOCAL_MODULE := font_res_images
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := $(_img_modules)
include $(BUILD_PHONY_PACKAGE)

_add-font-image :=
_img_modules :=
