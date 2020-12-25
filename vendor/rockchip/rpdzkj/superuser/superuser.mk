CUR_PATH := vendor/rockchip/rpdzkj/superuser/

PRODUCT_COPY_FILES += vendor/rockchip/rpdzkj/superuser/superuser.sh:vendor/bin/superuser.sh
PRODUCT_COPY_FILES += vendor/rockchip/rpdzkj/superuser/nosuperuser.sh:vendor/bin/nosuperuser.sh
modeswitch_files := $(shell ls $(CUR_PATH)/root_enable)
PRODUCT_COPY_FILES += \
    $(foreach file, $(modeswitch_files), \
    $(CUR_PATH)/root_enable/$(file):vendor/usr/superuser/root_enable/$(file))
modeswitch_files := $(shell ls $(CUR_PATH)/root_disable)
PRODUCT_COPY_FILES += \
    $(foreach file, $(modeswitch_files), \
    $(CUR_PATH)/root_disable/$(file):vendor/usr/superuser/root_disable/$(file))
