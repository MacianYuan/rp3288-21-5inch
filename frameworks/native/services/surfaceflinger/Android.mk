LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CLANG := true

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_SRC_FILES := \
    Client.cpp \
    DisplayDevice.cpp \
    DispSync.cpp \
    EventControlThread.cpp \
    StartPropertySetThread.cpp \
    EventThread.cpp \
    FrameTracker.cpp \
    GpuService.cpp \
    Layer.cpp \
    LayerDim.cpp \
    LayerRejecter.cpp \
    LayerVector.cpp \
    MessageQueue.cpp \
    MonitoredProducer.cpp \
    SurfaceFlingerConsumer.cpp \
    SurfaceInterceptor.cpp \
    Transform.cpp \
    DisplayHardware/ComposerHal.cpp \
    DisplayHardware/FramebufferSurface.cpp \
    DisplayHardware/HWC2.cpp \
    DisplayHardware/HWComposerBufferCache.cpp \
    DisplayHardware/PowerHAL.cpp \
    DisplayHardware/VirtualDisplaySurface.cpp \
    Effects/Daltonizer.cpp \
    EventLog/EventLogTags.logtags \
    EventLog/EventLog.cpp \
    RenderEngine/Description.cpp \
    RenderEngine/Mesh.cpp \
    RenderEngine/Program.cpp \
    RenderEngine/ProgramCache.cpp \
    RenderEngine/GLExtensions.cpp \
    RenderEngine/RenderEngine.cpp \
    RenderEngine/Texture.cpp \
    RenderEngine/GLES20RenderEngine.cpp \

LOCAL_MODULE := libsurfaceflinger
LOCAL_C_INCLUDES := \
    frameworks/native/vulkan/include \
    external/vulkan-validation-layers/libs/vkjson \
    system/libhwbinder/fast_msgq/include \

LOCAL_CFLAGS := -DLOG_TAG=\"SurfaceFlinger\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

########## For rockchip support. ##########
RK_SUPPORT := 1
LOCAL_CFLAGS_32 += -DARCH_32=1
LOCAL_CFLAGS_64 += -DARCH_64=1

LOCAL_CFLAGS += -DRK_SUPPORT=$(RK_SUPPORT)
ifeq ($(RK_SUPPORT),1)
RK_STEREO := 1
LOCAL_CFLAGS += -DRK_STEREO=$(RK_STEREO)

ifeq ($(strip $(TARGET_BOARD_PLATFORM_PRODUCT)),vr)
RK_VR := 1
else
RK_VR := 0
endif
LOCAL_CFLAGS += -DRK_VR=$(RK_VR)

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3288)
        LOCAL_CFLAGS += -DSF_RK3288
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3368)
        LOCAL_CFLAGS += -DSF_RK3368
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3366)
        LOCAL_CFLAGS += -DSF_RK3366
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3399)
        LOCAL_CFLAGS +=  -DSF_RK3399
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk322x)
        LOCAL_CFLAGS +=  -DSF_RK322X
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3128h)
        LOCAL_CFLAGS +=  -DSF_RK322X
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),sofia3gr)
LOCAL_CFLAGS += -DUSE_X86
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk30xxb)
    LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM_RK30XXB
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)),G6110)
        LOCAL_CFLAGS += -DGPU_G6110
endif

ifeq ($(strip $(BOARD_USE_DRM)),true)
RK_USE_DRM = 1
RK_USE_3_FB = 1
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3288)
RK_NV12_10_TO_NV12_BY_RGA = 0
RK_NV12_10_TO_NV12_BY_NENO = 1
RK_HDR = 0
else
ifneq ($(filter rk3399, $(strip $(TARGET_BOARD_PLATFORM))), )
RK_NV12_10_TO_NV12_BY_RGA = 0
RK_NV12_10_TO_NV12_BY_NENO = 0
RK_HDR = 1
LOCAL_CFLAGS += -DEECOLOR
LOCAL_C_INCLUDES += frameworks/native/libs/math/include
else
RK_NV12_10_TO_NV12_BY_RGA = 1
RK_NV12_10_TO_NV12_BY_NENO = 0
RK_HDR = 0
endif
endif
else
RK_HDR = 0
RK_USE_DRM = 0
RK_USE_3_FB = 0
ifneq ($(filter rk3328 rk3228h, $(strip $(TARGET_BOARD_PLATFORM))), )
RK_NV12_10_TO_NV12_BY_RGA = 1
RK_NV12_10_TO_NV12_BY_NENO = 0
else
RK_NV12_10_TO_NV12_BY_RGA = 0
RK_NV12_10_TO_NV12_BY_NENO = 0
endif
endif
LOCAL_CFLAGS += -DRK_USE_DRM=$(RK_USE_DRM) -DRK_USE_3_FB=$(RK_USE_3_FB) -DRK_USE_3_LAYER_BUFFER=1 \
		-DRK_BLACK_NV12_10_LAYER=0 -DRK_NV12_10_TO_NV12_BY_RGA=$(RK_NV12_10_TO_NV12_BY_RGA) \
	        -DRK_NV12_10_TO_NV12_BY_NENO=$(RK_NV12_10_TO_NV12_BY_NENO) -DRK_HDR=$(RK_HDR)

LOCAL_ARM_MODE := arm

endif
########## End of RK_SUPPORT ##########

ifeq ($(TARGET_USES_HWC2),true)
    LOCAL_CFLAGS += -DUSE_HWC2
    LOCAL_SRC_FILES += \
        SurfaceFlinger.cpp \
        DisplayHardware/HWComposer.cpp
    LOCAL_C_INCLUDES += \
	hardware/rockchip/librga
else
    LOCAL_SRC_FILES += \
        SurfaceFlinger_hwc1.cpp \
        DisplayHardware/HWComposer_hwc1.cpp
    LOCAL_C_INCLUDES += \
	hardware/rockchip/librga
endif

LOCAL_CFLAGS += -fvisibility=hidden -Werror=format

LOCAL_STATIC_LIBRARIES := \
    libhwcomposer-command-buffer \
    libtrace_proto \
    libvkjson \
    libvr_manager \
    libvrflinger

LOCAL_SHARED_LIBRARIES := \
    android.frameworks.vr.composer@1.0 \
    android.hardware.graphics.allocator@2.0 \
    android.hardware.graphics.composer@2.1 \
    android.hardware.configstore@1.0 \
    android.hardware.configstore-utils \
    libcutils \
    liblog \
    libdl \
    libfmq \
    libhardware \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libutils \
    libEGL \
    libGLESv1_CM \
    libGLESv2 \
    libbinder \
    libui \
    libgui \
    libpowermanager \
    libvulkan \
    libsync \
    libprotobuf-cpp-lite \
    libbase \
    android.hardware.power@1.0 \
    librga

LOCAL_EXPORT_SHARED_LIBRARY_HEADERS := \
    android.hardware.graphics.allocator@2.0 \
    android.hardware.graphics.composer@2.1 \
    libhidlbase \
    libhidltransport \
    libhwbinder

LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code -std=c++1z

include $(BUILD_SHARED_LIBRARY)

###############################################################
# build surfaceflinger's executable
include $(CLEAR_VARS)

LOCAL_CLANG := true

LOCAL_LDFLAGS_32 := -Wl,--version-script,art/sigchainlib/version-script32.txt -Wl,--export-dynamic
LOCAL_LDFLAGS_64 := -Wl,--version-script,art/sigchainlib/version-script64.txt -Wl,--export-dynamic
LOCAL_CFLAGS := -DLOG_TAG=\"SurfaceFlinger\"

LOCAL_INIT_RC := surfaceflinger.rc

ifeq ($(TARGET_USES_HWC2),true)
    LOCAL_CFLAGS += -DUSE_HWC2
endif

LOCAL_SRC_FILES := \
    main_surfaceflinger.cpp

LOCAL_SHARED_LIBRARIES := \
    android.frameworks.displayservice@1.0 \
    android.hardware.configstore@1.0 \
    android.hardware.configstore-utils \
    android.hardware.graphics.allocator@2.0 \
    libsurfaceflinger \
    libcutils \
    libdisplayservicehidl \
    liblog \
    libbinder \
    libhidlbase \
    libhidltransport \
    libutils \
    libui \
    libgui \
    libdl

LOCAL_WHOLE_STATIC_LIBRARIES := libsigchain
LOCAL_STATIC_LIBRARIES := libtrace_proto

LOCAL_MODULE := surfaceflinger

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

include $(BUILD_EXECUTABLE)

###############################################################
# uses jni which may not be available in PDK
ifneq ($(wildcard libnativehelper/include),)
include $(CLEAR_VARS)

LOCAL_CLANG := true

LOCAL_CFLAGS := -DLOG_TAG=\"SurfaceFlinger\"

LOCAL_SRC_FILES := \
    DdmConnection.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdl

LOCAL_MODULE := libsurfaceflinger_ddmconnection

LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

include $(BUILD_SHARED_LIBRARY)
endif # libnativehelper

include $(call first-makefiles-under,$(LOCAL_PATH))
