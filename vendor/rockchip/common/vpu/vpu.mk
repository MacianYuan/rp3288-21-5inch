LOCAL_PATH := $(call my-dir)

PRODUCT_PACKAGES += \
    libOMX_Core \
    libomxvpu_dec \
    libomxvpu_enc \
    librk_demux \
    librk_hevcdec \
    libRkOMX_Resourcemanager \
    librk_vpuapi

ifeq ($(filter sofia%, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_PACKAGES += \
    libhevcdec \
    librkwmapro \
    librkswscale \
    libjesancache \
    libapedec\
    librk_audio \
    wfd
else
ifeq ($(strip $(USE_INTEL_MDP)), true)
PRODUCT_PACKAGES += \
    libmdp_omx_core \
    libstagefright_soft_aacdec_mdp \
    libstagefright_soft_aacenc_mdp \
    libstagefright_soft_amrdec_mdp \
    libstagefright_soft_amrenc_mdp \
    libstagefright_soft_mp3dec_mdp \
    libstagefright_soft_vorbisdec_mdp
endif
endif

ifeq ($(filter rk2928, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_PACKAGES += \
    libjpeghwdec \
    libjpeghwenc
endif

ifneq ($(filter rk3228 rk3229 rk322x rk3128h rk3328, $(strip $(TARGET_BOARD_PLATFORM))), )
PRODUCT_COPY_FILES += \
	vendor/rockchip/common/vpu/lib/arm/rk322x/modules/vcodec_service.ko:$(TARGET_COPY_OUT_VENDOR)/lib/modules/vcodec_service.ko
PRODUCT_PACKAGES += \
	libiep 
endif

ifneq ($(filter rk3228 rk3229 rk322x rk3128h rk3328, $(strip $(TARGET_BOARD_PLATFORM))), )
PRODUCT_COPY_FILES += \
        vendor/rockchip/common/vpu/lib/arm/rk322x/libffmpeg_omx.so:$(TARGET_COPY_OUT_VENDOR)/lib/libffmpeg_omx.so \
        vendor/rockchip/common/vpu/lib/arm/rk322x/libffmpeg_utils.so:$(TARGET_COPY_OUT_VENDOR)/lib/libffmpeg_utils.so \
        vendor/rockchip/common/vpu/etc/media_codecs_ffmpeg_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_ffmpeg_video.xml \
        vendor/rockchip/common/vpu/etc/media_codecs_ffmpeg_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_ffmpeg_audio.xml
endif

ifneq ($(filter rk3399 rk3326, $(strip $(TARGET_BOARD_PLATFORM))), )
PRODUCT_COPY_FILES += \
	vendor/rockchip/common/vpu/lib/arm/rk3399/libjpeghwenc.so:$(TARGET_COPY_OUT_VENDOR)/lib/libjpeghwenc.so \
	vendor/rockchip/common/vpu/lib/arm/rk3399/libjpeghwdec.so:$(TARGET_COPY_OUT_VENDOR)/lib/libjpeghwdec.so \
	vendor/rockchip/common/vpu/lib/arm64/rk3399/libjpeghwenc.so:$(TARGET_COPY_OUT_VENDOR)/lib64/libjpeghwenc.so \
	vendor/rockchip/common/vpu/lib/arm64/rk3399/libjpeghwdec.so:$(TARGET_COPY_OUT_VENDOR)/lib64/libjpeghwdec.so
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3399)
PRODUCT_COPY_FILES += \
        vendor/rockchip/common/vpu/lib/arm/rk3399/libstagefright_hdcp.so:$(TARGET_COPY_OUT_VENDOR)/lib/libstagefright_hdcp.so \
        vendor/rockchip/common/vpu/lib/arm64/rk3399/libstagefright_hdcp.so:$(TARGET_COPY_OUT_VENDOR)/lib64/libstagefright_hdcp.so
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3128)
PRODUCT_COPY_FILES += \
	vendor/rockchip/common/vpu/lib/arm/libffmpeg_vp9dec.so:$(TARGET_COPY_OUT_VENDOR)/lib/libffmpeg_vp9dec.so 
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3366)
PRODUCT_COPY_FILES += \
	vendor/rockchip/common/vpu/lib/arm/rk3366/libjpeghwenc.so:$(TARGET_COPY_OUT_VENDOR)/lib/libjpeghwenc.so
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3288)
PRODUCT_COPY_FILES += \
	vendor/rockchip/common/vpu/lib/arm/rk3288/libjpeghwenc.so:$(TARGET_COPY_OUT_VENDOR)/lib/libjpeghwenc.so
endif

ifneq ($(filter rk%, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_PACKAGES += \
    librkffplayer \
    libffmpeg \
    libcsiconv

PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/bin/arm/iso:$(TARGET_COPY_OUT_VENDOR)/bin/iso	
endif

ifneq ($(filter rk3228 rk3229 rk322x rk3128h rk3288 rk3328 rk3126c rk3366 rk3368 rk3399 rk3326, $(strip $(TARGET_BOARD_PLATFORM))), )
PRODUCT_PACKAGES += \
    libmpp \
    libvpu \
    libvpu_secure
endif

ifneq ($(filter rk3328, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/firmware/monet.bin:$(TARGET_COPY_OUT_VENDOR)/etc/firmware/monet.bin \
    vendor/rockchip/common/vpu/lib/arm/libihton.so:$(TARGET_COPY_OUT_VENDOR)/lib/libihton.so
endif

ifneq ($(filter rk3128h, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/lib/arm/rk3128h/libihton.so:$(TARGET_COPY_OUT_VENDOR)/lib/libihton.so
endif

PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_rk_vpu.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_rk_vpu.xml

ifneq ($(filter rk312x rk3126c rk3128 rk3188 rk3326, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_google_audio_rk312x.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    vendor/rockchip/common/vpu/etc/media_codecs_google_video_rk312x.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml
else
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    vendor/rockchip/common/vpu/etc/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml
endif

ifneq ($(filter rk312x rk3126c rk3128 rk3188, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_rk312x.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else ifneq ($(filter rk3228 rk3366, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_mpp.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else ifneq ($(filter rk3399, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_rk3399.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else ifneq ($(filter rk3328, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_rk3328.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else ifneq ($(filter rk3368, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_rk3368.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else ifneq ($(filter rk3326, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_rk3326.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else ifneq ($(filter rk3288, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_rk3288.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else ifneq ($(filter rk322x, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_rk322x.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else ifneq ($(filter rk3128h, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance_rk3128h.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
else
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
endif

ifneq ($(filter rk312x rk3126c rk3128 rk3188, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_rk312x.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
else ifneq ($(filter rk3399, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_rk3399.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
else ifneq ($(filter rk3326, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_rk3326.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
else ifneq ($(filter rk3288, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_rk3288.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
else ifneq ($(filter rk3328 rk322x, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_rk3328.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
else ifneq ($(filter rk3128h, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_rk3128h.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
else ifneq ($(filter rk%, $(TARGET_BOARD_PLATFORM)), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
else
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/vpu/etc/media_codecs_sofia.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
endif

ifneq ($(filter px5%, $(PRODUCT_BUILD_MODULE)), )
PRODUCT_PACKAGES += \
     libdrivingrecoder
endif
