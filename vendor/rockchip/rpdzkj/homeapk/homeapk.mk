CUR_PATH := vendor/rockchip/rpdzkj/homeapk/

PRODUCT_COPY_FILES += vendor/rockchip/rpdzkj/homeapk/homeapk.sh:vendor/bin/homeapk.sh
PRODUCT_COPY_FILES += vendor/rockchip/rpdzkj/homeapk/nohomeapk.sh:vendor/bin/nohomeapk.sh
modeswitch_files := $(shell ls $(CUR_PATH)/Launcher3)
PRODUCT_COPY_FILES += \
    $(foreach file, $(modeswitch_files), \
    $(CUR_PATH)/Launcher3/$(file):vendor/usr/homeapk/Launcher3/$(file))
