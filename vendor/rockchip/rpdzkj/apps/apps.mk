CUR_PATH := vendor/rockchip/rpdzkj/apps


PRODUCT_COPY_FILES += vendor/rockchip/rpdzkj/apps/preinstall.sh:vendor/bin/preinstall.sh
modeswitch_files := $(shell ls $(CUR_PATH)/preinstall)
PRODUCT_COPY_FILES += \
    $(foreach file, $(modeswitch_files), \
    $(CUR_PATH)/preinstall/$(file):vendor/usr/preinstall/$(file))

PRODUCT_PACKAGES += \
    serialport
