# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# We're moving the emulator-specific platform libs to
# development.git/tools/emulator/. The following test is to ensure
# smooth builds even if the tree contains both versions.
#
#ifndef BUILD_EMULATOR_GPS_MODULE
#BUILD_EMULATOR_GPS_MODULE := true

LOCAL_PATH := $(call my-dir)

# HAL module implemenation stored in
# hw/<GPS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
#LOCAL_CFLAGS += -DQEMU_HARDWARE
LOCAL_CFLAGS += -Wunused
LOCAL_CFLAGS += -Wall
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := liblog libcutils libhardware
SRC_FILES_LIST := $(wildcard $(LOCAL_PATH)/*.c)
LOCAL_SRC_FILES := $(SRC_FILES_LIST:$(LOCAL_PATH)/%=%)

LOCAL_MODULE := gps.default

LOCAL_MODULE_TAGS := debug
include $(BUILD_SHARED_LIBRARY)

#endif # BUILD_EMULATOR_GPS_MODULE

############### agps/libtdctypto.so to /system/lib/ ############
include $(CLEAR_VARS)
LOCAL_MODULE := libtdcrypto.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_SRC_FILES := AGNSS/libtdcrypto.so
include $(BUILD_PREBUILT)

############### agps/libtdssl.so to /system/lib/ ############
include $(CLEAR_VARS)
LOCAL_MODULE := libtdssl.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_SRC_FILES := AGNSS/libtdssl.so
include $(BUILD_PREBUILT)

############### agps/libtdsupl.so /system/lib/ ############
include $(CLEAR_VARS)
LOCAL_MODULE := libtdsupl.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_SRC_FILES := AGNSS/libtdsupl.so
include $(BUILD_PREBUILT)

############### agps/supl-client /system/bin/ ############
include $(CLEAR_VARS)
LOCAL_MODULE := supl-client
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECTUABLES)
LOCAL_SRC_FILES := AGNSS/supl-client
include $(BUILD_PREBUILT)

