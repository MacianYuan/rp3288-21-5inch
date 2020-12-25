# Copyright (C) 2010 Chia-I Wu <olvaffe@gmail.com>
# Copyright (C) 2010-2011 LunarG Inc.
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# Android.mk for drm_gralloc

DRM_GPU_DRIVERS := $(strip $(filter-out swrast, $(BOARD_GPU_DRIVERS)))
DRM_GPU_DRIVERS := rockchip
intel_drivers := i915 i965 i915g ilo
radeon_drivers := r300g r600g
rockchip_drivers := rockchip
nouveau_drivers := nouveau
vmwgfx_drivers := vmwgfx

valid_drivers := \
	prebuilt \
	$(intel_drivers) \
	$(radeon_drivers) \
	$(rockchip_drivers) \
	$(nouveau_drivers) \
	$(vmwgfx_drivers)

# warn about invalid drivers
invalid_drivers := $(filter-out $(valid_drivers), $(DRM_GPU_DRIVERS))
ifneq ($(invalid_drivers),)
$(warning invalid GPU drivers: $(invalid_drivers))
# tidy up
DRM_GPU_DRIVERS := $(filter-out $(invalid_drivers), $(DRM_GPU_DRIVERS))
endif

ifneq ($(filter $(vmwgfx_drivers), $(DRM_GPU_DRIVERS)),)
DRM_USES_PIPE := true
else
DRM_USES_PIPE := false
endif

ifneq ($(strip $(DRM_GPU_DRIVERS)),)

LOCAL_PATH := $(call my-dir)


# Use the PREBUILT libraries
ifeq ($(strip $(DRM_GPU_DRIVERS)),prebuilt)

include $(CLEAR_VARS)
LOCAL_MODULE := libgralloc_drm
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../$(BOARD_GPU_DRIVER_BINARY)
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := gralloc.$(TARGET_PRODUCT)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := ../../$(BOARD_GPU_DRIVER_BINARY)
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
include $(BUILD_PREBUILT)

# Use the sources
else

include $(CLEAR_VARS)
LOCAL_MODULE := libgralloc_drm
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
endif
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	gralloc_drm.cpp

LOCAL_C_INCLUDES := \
	external/libdrm \
	external/libdrm/include/drm

LOCAL_SHARED_LIBRARIES := \
	libdrm \
	liblog \
	libcutils \
	libutils

ifneq ($(filter $(intel_drivers), $(DRM_GPU_DRIVERS)),)
LOCAL_SRC_FILES += gralloc_drm_intel.c
LOCAL_C_INCLUDES += external/libdrm/intel
LOCAL_CFLAGS += -DENABLE_INTEL
LOCAL_SHARED_LIBRARIES += libdrm_intel
endif

ifneq ($(filter $(radeon_drivers), $(DRM_GPU_DRIVERS)),)
LOCAL_SRC_FILES += gralloc_drm_radeon.c
LOCAL_C_INCLUDES += external/libdrm/radeon
LOCAL_CFLAGS += -DENABLE_RADEON
LOCAL_SHARED_LIBRARIES += libdrm_radeon
endif

ifneq ($(filter $(nouveau_drivers), $(DRM_GPU_DRIVERS)),)
LOCAL_SRC_FILES += gralloc_drm_nouveau.c
LOCAL_C_INCLUDES += external/libdrm/nouveau
LOCAL_CFLAGS += -DENABLE_NOUVEAU
LOCAL_SHARED_LIBRARIES += libdrm_nouveau
endif

ifneq ($(filter $(rockchip_drivers), $(DRM_GPU_DRIVERS)),)
MALI_ARCHITECTURE_UTGARD := 0
MALI_SUPPORT_AFBC_WIDEBLK?=0
MALI_USE_YUV_AFBC_WIDEBLK?=0
# AFBC buffers should be initialised after allocation in all rk platforms.
GRALLOC_INIT_AFBC?=1

# not use afbc layer by default.
USE_AFBC_LAYER = 0

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3399)
USE_AFBC_LAYER = 1
ifeq ($(strip $(TARGET_BOARD_PLATFORM_PRODUCT)),box)
USE_AFBC_LAYER = 0
endif
endif

# enable AFBC by default
MALI_AFBC_GRALLOC := 1

DISABLE_FRAMEBUFFER_HAL?=1
GRALLOC_DEPTH?=GRALLOC_32_BITiS
GRALLOC_FB_SWAP_RED_BLUE?=0

#If cropping should be enabled for AFBC YUV420 buffers
AFBC_YUV420_EXTRA_MB_ROW_NEEDED ?= 0

ifeq ($(GRALLOC_ARM_NO_EXTERNAL_AFBC),1)
LOCAL_CFLAGS += -DGRALLOC_ARM_NO_EXTERNAL_AFBC=1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t720)
MALI_AFBC_GRALLOC := 0
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T72X=1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t760)
MALI_AFBC_GRALLOC := 1
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T76X=1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t860)
MALI_AFBC_GRALLOC := 1
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T86X=1
endif

ifeq ($(MALI_AFBC_GRALLOC), 1)
AFBC_FILES = gralloc_buffer_priv.cpp
else
MALI_AFBC_GRALLOC := 0
AFBC_FILES =
endif

LOCAL_C_INCLUDES += hardware/rockchip/librkvpu
LOCAL_SRC_FILES += gralloc_drm_rockchip.cpp \
	mali_gralloc_formats.cpp \
	$(AFBC_FILES)

LOCAL_SHARED_LIBRARIES += libdrm_rockchip

#RK_DRM_GRALLOC for rockchip drm gralloc
#RK_DRM_GRALLOC_DEBUG for rockchip drm gralloc debug.
MAJOR_VERSION := "RK_GRAPHICS_VER=commit-id:$(shell cd $(LOCAL_PATH) && git log  -1 --oneline | awk '{print $$1}')"
LOCAL_CFLAGS +=-DRK_DRM_GRALLOC=1 -DRK_DRM_GRALLOC_DEBUG=0 -DENABLE_ROCKCHIP -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION) -D$(GRALLOC_DEPTH) -DMALI_ARCHITECTURE_UTGARD=$(MALI_ARCHITECTURE_UTGARD) -DDISABLE_FRAMEBUFFER_HAL=$(DISABLE_FRAMEBUFFER_HAL) -DMALI_AFBC_GRALLOC=$(MALI_AFBC_GRALLOC) -DMALI_SUPPORT_AFBC_WIDEBLK=$(MALI_SUPPORT_AFBC_WIDEBLK) -DAFBC_YUV420_EXTRA_MB_ROW_NEEDED=$(AFBC_YUV420_EXTRA_MB_ROW_NEEDED) -DGRALLOC_INIT_AFBC=$(GRALLOC_INIT_AFBC) -DRK_GRAPHICS_VER=\"$(MAJOR_VERSION)\" -DUSE_AFBC_LAYER=$(USE_AFBC_LAYER)

ifeq ($(TARGET_USES_HWC2),true)
    LOCAL_CFLAGS += -DUSE_HWC2
endif

# disable arm_format_selection on rk platforms, by default.
LOCAL_CFLAGS += -DGRALLOC_ARM_FORMAT_SELECTION_DISABLE
LOCAL_CFLAGS += -DGRALLOC_LIBRARY_BUILD=1 -DGRALLOC_USE_GRALLOC1_API=0

ifeq ($(GRALLOC_FB_SWAP_RED_BLUE),1)
LOCAL_CFLAGS += -DGRALLOC_FB_SWAP_RED_BLUE
endif

ifeq ($(GRALLOC_ARM_NO_EXTERNAL_AFBC),1)
LOCAL_CFLAGS += -DGRALLOC_ARM_NO_EXTERNAL_AFBC=1
endif

ifdef PLATFORM_CFLAGS
LOCAL_CFLAGS += $(PLATFORM_CFLAGS)
endif

endif

ifeq ($(strip $(DRM_USES_PIPE)),true)
LOCAL_SRC_FILES += gralloc_drm_pipe.c
LOCAL_CFLAGS += -DENABLE_PIPE
LOCAL_C_INCLUDES += \
	external/mesa/include \
	external/mesa/src/gallium/include \
	external/mesa/src/gallium/winsys \
	external/mesa/src/gallium/drivers \
	external/mesa/src/gallium/auxiliary

ifneq ($(filter r600g, $(DRM_GPU_DRIVERS)),)
LOCAL_CFLAGS += -DENABLE_PIPE_R600
LOCAL_STATIC_LIBRARIES += \
	libmesa_pipe_r600 \
	libmesa_pipe_radeon \
	libmesa_winsys_radeon
endif
ifneq ($(filter vmwgfx, $(DRM_GPU_DRIVERS)),)
LOCAL_CFLAGS += -DENABLE_PIPE_VMWGFX
LOCAL_STATIC_LIBRARIES += \
	libmesa_pipe_svga \
	libmesa_winsys_svga
LOCAL_C_INCLUDES += \
	external/mesa/src/gallium/drivers/svga/include
endif

LOCAL_STATIC_LIBRARIES += \
	libmesa_gallium
LOCAL_SHARED_LIBRARIES += libdl
endif # DRM_USES_PIPE
include $(BUILD_SHARED_LIBRARY)

# ------------ #

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	gralloc.cpp


LOCAL_C_INCLUDES := \
	external/libdrm \
	external/libdrm/include/drm

LOCAL_SHARED_LIBRARIES := \
	libgralloc_drm \
	liblog \
	libutils

# for glFlush/glFinish
LOCAL_SHARED_LIBRARIES += \
	libGLESv1_CM
LOCAL_CFLAGS +=-DRK_DRM_GRALLOC=1 -DRK_DRM_GRALLOC_DEBUG=0 -DMALI_AFBC_GRALLOC=1 -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

ifeq ($(TARGET_USES_HWC2),true)
    LOCAL_CFLAGS += -DUSE_HWC2
endif

LOCAL_MODULE := gralloc.$(TARGET_BOARD_HARDWARE)
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
endif
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
include $(BUILD_SHARED_LIBRARY)

endif # DRM_GPU_DRIVERS=prebuilt
endif # DRM_GPU_DRIVERS
