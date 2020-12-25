ifneq ($(filter rk3326, $(strip $(TARGET_BOARD_PLATFORM))), )
OPTEE_DIR := vendor/rockchip/common/security/optee/v2
PRODUCT_COPY_FILES += \
	$(OPTEE_DIR)/ta/258be795-f9ca-40e6-a869-9ce6886c5d5d.ta:vendor/lib/optee_armtz/258be795-f9ca-40e6-a869-9ce6886c5d5d.ta	\
	$(OPTEE_DIR)/ta/0b82bae5-0cd0-49a5-9521-516dba9c43ba.ta:vendor/lib/optee_armtz/0b82bae5-0cd0-49a5-9521-516dba9c43ba.ta
else
OPTEE_DIR := vendor/rockchip/common/security/optee/v1
PRODUCT_COPY_FILES += \
	$(OPTEE_DIR)/ta/258be795-f9ca-40e6-a8699ce6886c5d5d.ta:vendor/lib/optee_armtz/258be795-f9ca-40e6-a8699ce6886c5d5d.ta	\
	$(OPTEE_DIR)/ta/0b82bae5-0cd0-49a5-9521516dba9c43ba.ta:vendor/lib/optee_armtz/0b82bae5-0cd0-49a5-9521516dba9c43ba.ta
endif

PRODUCT_COPY_FILES += \
	$(OPTEE_DIR)/lib/arm/libkeymaster2.so:vendor/lib/libkeymaster2.so	\
	$(OPTEE_DIR)/lib/arm/libRkTeeKeymaster.so:vendor/lib/libRkTeeKeymaster.so	\
	$(OPTEE_DIR)/lib/arm/libkeymaster_messages2.so:vendor/lib/libkeymaster_messages2.so	\
	$(OPTEE_DIR)/lib/arm/keystore.rk30board.so:vendor/lib/hw/keystore.rk30board.so	\
	$(OPTEE_DIR)/lib/arm/libRkTeeGatekeeper.so:vendor/lib/libRkTeeGatekeeper.so	\
	$(OPTEE_DIR)/lib/arm/librkgatekeeper.so:vendor/lib/librkgatekeeper.so	\
	$(OPTEE_DIR)/lib/arm/gatekeeper.rk30board.so:vendor/lib/hw/gatekeeper.rk30board.so	

ifeq ($(strip $(TARGET_ARCH)), arm64)
PRODUCT_COPY_FILES += \
	$(OPTEE_DIR)/lib/arm64/libkeymaster2.so:vendor/lib64/libkeymaster2.so	\
	$(OPTEE_DIR)/lib/arm64/libRkTeeKeymaster.so:vendor/lib64/libRkTeeKeymaster.so	\
	$(OPTEE_DIR)/lib/arm64/libkeymaster_messages2.so:vendor/lib64/libkeymaster_messages2.so	\
	$(OPTEE_DIR)/lib/arm64/keystore.rk30board.so:vendor/lib64/hw/keystore.rk30board.so	\
	$(OPTEE_DIR)/lib/arm64/libRkTeeGatekeeper.so:vendor/lib64/libRkTeeGatekeeper.so	\
	$(OPTEE_DIR)/lib/arm64/librkgatekeeper.so:vendor/lib64/librkgatekeeper.so	\
	$(OPTEE_DIR)/lib/arm64/gatekeeper.rk30board.so:vendor/lib64/hw/gatekeeper.rk30board.so
endif

ifeq ($(strip $(TARGET_ARCH)), arm64)
PRODUCT_COPY_FILES += \
	$(OPTEE_DIR)/lib/arm64/tee-supplicant:vendor/bin/tee-supplicant        \
	$(OPTEE_DIR)/lib/arm64/libteec.so:vendor/lib64/libteec.so
else
PRODUCT_COPY_FILES += \
        $(OPTEE_DIR)/lib/arm/tee-supplicant:vendor/bin/tee-supplicant
endif
PRODUCT_COPY_FILES += \
        $(OPTEE_DIR)/lib/arm/libteec.so:vendor/lib/libteec.so

# new gatekeeper HAL
PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0-impl \
    android.hardware.gatekeeper@1.0-service
