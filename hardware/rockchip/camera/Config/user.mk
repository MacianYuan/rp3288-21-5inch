ifeq ($(filter box vr stbvr, $(strip $(TARGET_BOARD_PLATFORM_PRODUCT))), )

PRODUCT_PACKAGES += \
    libisp_isi_drv_OV2659 \
	libisp_isi_drv_OV8825 \
	libisp_isi_drv_OV8820 \
	libisp_isi_drv_OV8858 \
	libisp_isi_drv_GS8604 \
	libisp_isi_drv_OV5648 \
	libisp_isi_drv_OV5640 \
	libisp_isi_drv_OV13850 \
	libisp_isi_drv_IMX214 \
	libisp_isi_drv_HM2057 \
	libisp_isi_drv_HM5040 \
	libisp_isi_drv_SP2518 \
	libisp_isi_drv_GC0308 \
	libisp_isi_drv_GC2035 \
	libisp_isi_drv_GC2145 \
	libisp_isi_drv_GC2355 \
	libisp_isi_drv_GC2155 \
	libisp_isi_drv_NT99252 \
	libisp_isi_drv_OV2680 \
	libisp_isi_drv_OV5645 \
	libisp_isi_drv_OV5695 \
	libisp_isi_drv_TC358749XBG \
	libisp_isi_drv_OV2685

endif
