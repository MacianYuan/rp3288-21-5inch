###############################################################################
# GPS HAL libraries
LOCAL_PATH := $(call my-dir)

PRODUCT_COPY_FILES += \
			hardware/rockchip/gps/TD1030HAL/AGNSS/libtdcrypto.so:vendor/lib/libtdcrypto.so \
			hardware/rockchip/gps/TD1030HAL/AGNSS/libtdssl.so:vendor/lib/libtdssl.so \
			hardware/rockchip/gps/TD1030HAL/AGNSS/libtdsupl.so:vendor/lib/libtdsupl.so \
			hardware/rockchip/gps/TD1030HAL/AGNSS/supl-client:vendor/bin/supl-client \
			hardware/rockchip/gps/TD1030HAL/AGNSS/tdgnss.conf:vendor/etc/tdgnss.conf


