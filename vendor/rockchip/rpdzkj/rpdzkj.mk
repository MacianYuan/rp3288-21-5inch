CUR_PATH := vendor/rockchip/rpdzkj/

-include $(CUR_PATH)/catlog/catlog.mk
-include $(CUR_PATH)/hdmiin/hdmiin.mk
include $(CUR_PATH)/superuser/superuser.mk
CUR_PATH := vendor/rockchip/rpdzkj/
include $(CUR_PATH)/apps/apps.mk
CUR_PATH := vendor/rockchip/rpdzkj/
include $(CUR_PATH)/homeapk/homeapk.mk
