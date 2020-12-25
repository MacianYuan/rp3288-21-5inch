#
# Copyright (C) 2014 The Android Open Source Project
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
#

LOCAL_PATH := $(call my-dir)

#########################################
# The prebuilt support libraries.

# For apps (unbundled) build, replace the typical
# make target artifacts with prebuilts.
ifneq (,$(TARGET_BUILD_APPS)$(filter true,$(TARGET_BUILD_PDK)))
include $(CLEAR_VARS)

# Set up prebuilts for the core Support Library artifacts.
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += \
  $(patsubst $(LOCAL_PATH)/%,%,\
    $(shell find $(LOCAL_PATH)/support -name "*.jar"))

# Set up prebuilts for additional non-core library artifacts.
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += \
  $(patsubst $(LOCAL_PATH)/%,%,\
    $(shell find $(LOCAL_PATH)/extras -name "*.jar"))

# Set up prebuilts for Multidex library artifacts.
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += \
  $(patsubst $(LOCAL_PATH)/%,%,\
    $(shell find $(LOCAL_PATH)/multidex -name "*.jar"))

include $(BUILD_MULTI_PREBUILT)

# Generates the v4, v13, and appcompat libraries with static dependencies.
include $(call all-makefiles-under,$(LOCAL_PATH))

endif  # TARGET_BUILD_APPS not empty or TARGET_BUILD_PDK set to True
