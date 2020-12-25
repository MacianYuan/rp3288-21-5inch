ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali400)
ifeq ($(strip $(TARGET_ARCH)), arm)
PRODUCT_PROPERTY_OVERRIDES += ro.sf.lcdc_composer=0
PRODUCT_PROPERTY_OVERRIDES += debug.hwui.render_dirty_regions=false
ifeq ($(strip $(GRAPHIC_MEMORY_PROVIDER)), dma_buf)

ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3188)
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/gpu/Mali400/lib/$(TARGET_ARCH)/rk3188/libGLES_mali.so:$(TARGET_COPY_OUT_VENDOR)/lib/egl/libGLES_mali.so \
    vendor/rockchip/common/gpu/Mali400/lib/$(TARGET_ARCH)/rk3188/libGLES_mali.so:obj/lib/libGLES_mali.so
else
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/gpu/Mali400/lib/$(TARGET_ARCH)/libGLES_mali.so:$(TARGET_COPY_OUT_VENDOR)/lib/egl/libGLES_mali.so \
    vendor/rockchip/common/gpu/Mali400/lib/$(TARGET_ARCH)/libGLES_mali.so:obj/lib/libGLES_mali.so
endif

ifneq ($(filter rk3036 rk3188, $(strip $(TARGET_BOARD_PLATFORM))), )
PRODUCT_COPY_FILES += \
    vendor/rockchip/common/gpu/Mali400/modules/$(TARGET_ARCH)/$(strip $(TARGET_BOARD_PLATFORM))/mali.ko:$(TARGET_COPY_OUT_VENDOR)/lib/modules/mali.ko
endif

PRODUCT_COPY_FILES += \
    vendor/rockchip/common/gpu/gpu_performance/etc/performance_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/performance_info.xml \
    vendor/rockchip/common/gpu/gpu_performance/etc/packages-compat.xml:$(TARGET_COPY_OUT_VENDOR)/etc/packages-compat.xml \
    vendor/rockchip/common/gpu/gpu_performance/etc/packages-composer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/packages-composer.xml \
    vendor/rockchip/common/gpu/gpu_performance/bin/$(TARGET_ARCH)/performance:$(TARGET_COPY_OUT_VENDOR)/bin/performance \
    vendor/rockchip/common/gpu/gpu_performance/lib/$(TARGET_ARCH)/libperformance_runtime.so:$(TARGET_COPY_OUT_VENDOR)/lib/libperformance_runtime.so \
    vendor/rockchip/common/gpu/gpu_performance/lib/$(TARGET_ARCH)/gpu.$(TARGET_BOARD_HARDWARE).so:$(TARGET_COPY_OUT_VENDOR)/lib/hw/gpu.$(TARGET_BOARD_HARDWARE).so

endif
endif
endif
