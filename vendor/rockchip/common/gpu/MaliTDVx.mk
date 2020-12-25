ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-tDVx)

ifeq ($(strip $(TARGET_ARCH)), arm64)
PRODUCT_COPY_FILES += \
	vendor/rockchip/common/gpu/MaliTDVx/lib/modules/mali_kbase.ko:$(TARGET_COPY_OUT_VENDOR)/lib/modules/mali_kbase.ko \
	vendor/rockchip/common/gpu/MaliTDVx/lib/arm64/libGLES_mali.so:$(TARGET_COPY_OUT_VENDOR)/lib64/egl/libGLES_mali.so \
	vendor/rockchip/common/gpu/MaliTDVx/lib/arm/libGLES_mali.so:$(TARGET_COPY_OUT_VENDOR)/lib/egl/libGLES_mali.so
endif

endif
