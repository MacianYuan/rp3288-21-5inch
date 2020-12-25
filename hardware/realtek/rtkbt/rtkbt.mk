# RELEASE NAME: 20171107_BT_ANDROID_8.x

BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_RTK := true
BOARD_HAVE_BLUETOOTH_RTK_COEX := true


BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(LOCAL_PATH)/bluetooth


#PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/vendor/etc/bluetooth/rtkbt.conf:vendor/etc/bluetooth/rtkbt.conf \
	$(LOCAL_PATH)/vendor/firmware/rtl8703as_config:vendor/firmware/rtl8703as_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8703as_fw:vendor/firmware/rtl8703as_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8703bs_config:vendor/firmware/rtl8703bs_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8703bs_fw:vendor/firmware/rtl8703bs_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8703cs_config:vendor/firmware/rtl8703cs_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8703cs_fw:vendor/firmware/rtl8703cs_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723a_config:vendor/firmware/rtl8723a_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723a_config_addr:vendor/firmware/rtl8723a_config_addr \
	$(LOCAL_PATH)/vendor/firmware/rtl8723a_fw:vendor/firmware/rtl8723a_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723as_config:vendor/firmware/rtl8723as_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723as_fw:vendor/firmware/rtl8723as_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723b_config:vendor/firmware/rtl8723b_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723b_config_2Ant_S0:vendor/firmware/rtl8723b_config_2Ant_S0 \
	$(LOCAL_PATH)/vendor/firmware/rtl8723b_fw:vendor/firmware/rtl8723b_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723bs_config:vendor/firmware/rtl8723bs_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723bs_fw:vendor/firmware/rtl8723bs_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723bs_VQ0_config:vendor/firmware/rtl8723bs_VQ0_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723bs_VQ0_fw:vendor/firmware/rtl8723bs_VQ0_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723bu_config:vendor/firmware/rtl8723bu_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723c_fw:vendor/firmware/rtl8723c_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723cs_cg_config:vendor/firmware/rtl8723cs_cg_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723cs_cg_fw:vendor/firmware/rtl8723cs_cg_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723cs_vf_config:vendor/firmware/rtl8723cs_vf_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723cs_vf_fw:vendor/firmware/rtl8723cs_vf_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723cs_xx_config:vendor/firmware/rtl8723cs_xx_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723cs_xx_fw:vendor/firmware/rtl8723cs_xx_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723d_config:vendor/firmware/rtl8723d_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723d_fw:vendor/firmware/rtl8723d_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8723ds_config:vendor/firmware/rtl8723ds_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8723ds_fw:vendor/firmware/rtl8723ds_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8761a_config:vendor/firmware/rtl8761a_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8761at8192ee_fw:vendor/firmware/rtl8761at8192ee_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8761at_config:vendor/firmware/rtl8761at_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8761at_fw:vendor/firmware/rtl8761at_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8761au8192ee_fw:vendor/firmware/rtl8761au8192ee_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8761au8812ae_fw:vendor/firmware/rtl8761au8812ae_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8761au_fw:vendor/firmware/rtl8761au_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8761aw8192eu_config:vendor/firmware/rtl8761aw8192eu_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8761aw8192eu_fw:vendor/firmware/rtl8761aw8192eu_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8821a_config:vendor/firmware/rtl8821a_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8821a_fw:vendor/firmware/rtl8821a_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8821as_config:vendor/firmware/rtl8821as_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8821as_fw:vendor/firmware/rtl8821as_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8821c_config:vendor/firmware/rtl8821c_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8821c_fw:vendor/firmware/rtl8821c_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8821cs_config:vendor/firmware/rtl8821cs_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8821cs_fw:vendor/firmware/rtl8821cs_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8822b_config:vendor/firmware/rtl8822b_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8822b_fw:vendor/firmware/rtl8822b_fw \
	$(LOCAL_PATH)/vendor/firmware/rtl8822bs_config:vendor/firmware/rtl8822bs_config \
	$(LOCAL_PATH)/vendor/firmware/rtl8822bs_fw:vendor/firmware/rtl8822bs_fw \

CUR_PATH := hardware/realtek/rtkbt
PRODUCT_COPY_FILES += \
	$(CUR_PATH)/vendor/etc/bluetooth/rtkbt.conf:vendor/etc/bluetooth/rtkbt.conf

BT_FIRMWARE_FILES := $(shell ls $(CUR_PATH)/vendor/firmware)
PRODUCT_COPY_FILES += \
	$(foreach file, $(BT_FIRMWARE_FILES), $(CUR_PATH)/vendor/firmware/$(file):$(TARGET_COPY_OUT_VENDOR)/etc/firmware/$(file))

PRODUCT_PACKAGES += \
	Bluetooth \
	libbt-vendor \
	android.hardware.bluetooth@1.0-impl \
	android.hidl.memory@1.0-impl \
	android.hardware.bluetooth@1.0-service \
	android.hardware.bluetooth@1.0-service.rc \
	audio.a2dp.default


PRODUCT_PROPERTY_OVERRIDES += \
	persist.bluetooth.btsnoopenable=false \
	persist.bluetooth.btsnooppath=/sdcard/btsnoop_hci.cfa \
	persist.bluetooth.btsnoopsize=0xffff \
	persist.bluetooth.rtkcoex=true \
	bluetooth.enable_timeout_ms=11000


