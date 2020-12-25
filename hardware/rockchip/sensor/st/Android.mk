LOCAL_PATH := $(call my-dir)

# HAL module implemenation, not prelinked, and stored in
# hw/<SENSORS_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

LOCAL_MULTILIB := both

LOCAL_CFLAGS += \
	-Wno-unused-parameter

LOCAL_CPPFLAGS += \
	-Wno-unused-parameter

ifeq ($(BOARD_GRAVITY_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DGRAVITY_SENSOR_SUPPORT
endif

ifeq ($(BOARD_COMPASS_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DCOMPASS_SENSOR_SUPPORT
endif

ifeq ($(BOARD_GYROSCOPE_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DGYROSCOPE_SENSOR_SUPPORT
endif

ifeq ($(BOARD_PROXIMITY_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DPROXIMITY_SENSOR_SUPPORT
endif

ifeq ($(BOARD_LIGHT_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DLIGHT_SENSOR_SUPPORT
endif

ifeq ($(BOARD_PRESSURE_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DPRESSURE_SENSOR_SUPPORT
endif

ifeq ($(BOARD_TEMPERATURE_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DTEMPERATURE_SENSOR_SUPPORT
endif

LOCAL_MODULE := sensors.$(TARGET_BOARD_HARDWARE)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -DLOG_TAG=\"SensorsHal\" \
	-DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_SRC_FILES := \
	sensors.c \
	nusensors.cpp \
	GyroSensor.cpp \
	InputEventReader.cpp \
	SensorBase.cpp \
	AkmSensor.cpp \
	MmaSensor.cpp \
	LightSensor.cpp \
	ProximitySensor.cpp \
	PressureSensor.cpp \
	TemperatureSensor.cpp
				
LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libutils

include $(BUILD_SHARED_LIBRARY)

######### AKM daemon #################################################

ifeq ($(strip $(BOARD_SENSOR_COMPASS_AK8963)), true)
include $(LOCAL_PATH)/akm8963/Android.mk
else ifeq ($(strip $(BOARD_SENSOR_COMPASS_AK09911)), true)
include $(LOCAL_PATH)/akm09911/Android.mk
else ifeq ($(strip $(BOARD_SENSOR_COMPASS_AK8975)), true)
include $(LOCAL_PATH)/akm8975/Android.mk
else ifeq ($(strip $(BOARD_SENSOR_COMPASS_AK8963-64)), true)
include $(LOCAL_PATH)/akm8963-64/Android.mk
endif # ifeq ($(strip $(BOARD_SENSOR_COMPASS_AK8963)), true)
