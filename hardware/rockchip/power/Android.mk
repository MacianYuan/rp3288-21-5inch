#--------------------------------------------------------------------------
#  Copyright (C) 2016 Fuzhou Rockchip Electronics Co. Ltd. All rights reserved.

#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#    * Neither the name of The Linux Foundation nor
#      the names of its contributors may be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.

#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
#ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#--------------------------------------------------------------------------



LOCAL_PATH := $(call my-dir)

# HAL module implemenation stored in
# hw/<POWERS_HARDWARE_MODULE_ID>.<ro.board.platform>.so
include $(CLEAR_VARS)

ifneq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 5.0)))
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
else
ifneq ($(strip $(TARGET_2ND_ARCH)), )
LOCAL_MULTILIB := both
endif
LOCAL_MODULE_RELATIVE_PATH := hw
endif

LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := power.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM=\"$(TARGET_BOARD_PLATFORM)\"
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM_PRODUCT=\"$(TARGET_BOARD_PLATFORM_PRODUCT)\"

ifeq ($(strip $(BOARD_DDR_VAR_ENABLED)),true)
LOCAL_CFLAGS += -DDDR_BOOST_SUPPORT=1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3399)
LOCAL_SRC_FILES := power_rk3399.c
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3368)
LOCAL_SRC_FILES := power_rk3368.c
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3326)
LOCAL_SRC_FILES := power_rk3326.c
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3126c)
LOCAL_SRC_FILES := power_rk312x.c
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3288)
LOCAL_SRC_FILES := power_rk3288.c
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk322x)
LOCAL_SRC_FILES := power_rk322x.c
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3128h)
LOCAL_SRC_FILES := power_rk322x.c
endif
#TODO
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3328)
LOCAL_SRC_FILES := power_rk3328.c
endif

ifeq ($(strip $(PRODUCT_BUILD_MODULE)),px5car)
LOCAL_CFLAGS += -DRK3368_PX5CAR
endif

include $(BUILD_SHARED_LIBRARY)
