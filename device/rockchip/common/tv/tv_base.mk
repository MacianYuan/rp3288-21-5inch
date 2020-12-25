#
# Copyright (C) 2014 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

PRODUCT_PACKAGES := \
    TvProvider \
    TvSettings \
    tv_input.default

PRODUCT_COPY_FILES := \
    device/rockchip/common/tv/permissions/tv_core_hardware.xml:system/etc/permissions/tv_core_hardware.xml

DEVICE_PACKAGE_OVERLAYS := \
    device/rockchip/common/tv/overlay

# From build/target/product/core_base.mk
PRODUCT_PACKAGES += \
    ContactsProvider \
    DefaultContainerService \
    UserDictionaryProvider \
    libaudiopreprocessing \
    libfilterpack_imageproc \
    libgabi++ \
    libkeystore \
    libstagefright_soft_aacdec \
    libstagefright_soft_aacenc \
    libstagefright_soft_amrdec \
    libstagefright_soft_amrnbenc \
    libstagefright_soft_amrwbenc \
    libstagefright_soft_avcdec \
    libstagefright_soft_avcenc \
    libstagefright_soft_flacdec \
    libstagefright_soft_flacenc \
    libstagefright_soft_g711dec \
    libstagefright_soft_gsmdec \
    libstagefright_soft_hevcdec \
    libstagefright_soft_mp3dec \
    libstagefright_soft_mpeg2dec \
    libstagefright_soft_mpeg4dec \
    libstagefright_soft_mpeg4enc \
    libstagefright_soft_opusdec \
    libstagefright_soft_rawdec \
    libstagefright_soft_vorbisdec \
    libstagefright_soft_vpxdec \
    libstagefright_soft_vpxenc \
    mdnsd \
    requestsync

# From build/target/product/core.mk
PRODUCT_PACKAGES += \
    BasicDreams \
    Browser \
    CalendarProvider \
    ExactCalculator \
    CaptivePortalLogin \
    CertInstaller \
    ExternalStorageProvider \
    FusedLocation \
    InputDevices \
    KeyChain \
    LatinIME \
    PicoTts \
    PacProcessor \
    PrintSpooler \
    ProxyHandler \
    SharedStorageBackup \
    VpnDialogs \
    com.android.media.tv.remoteprovider \
    com.android.media.tv.remoteprovider.xml

# From build/target/product/generic_no_telephony.mk
PRODUCT_PACKAGES += \
    Bluetooth \
    SystemUI \
    librs_jni \
    audio.primary.default \
    audio_policy.default \
    clatd \
    clatd.conf \
    local_time.default \
    screenrecord \
    Camera2 \
    Provision

PRODUCT_COPY_FILES += \
    frameworks/av/media/libeffects/data/audio_effects.conf:system/etc/audio_effects.conf

# Enable frame-exact AV sync
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.media.avsync=true

$(call inherit-product-if-exists, frameworks/base/data/sounds/AllAudio.mk)
$(call inherit-product-if-exists, external/svox/pico/lang/all_pico_languages.mk)
$(call inherit-product-if-exists, frameworks/base/data/fonts/fonts.mk)
$(call inherit-product-if-exists, external/google-fonts/dancing-script/fonts.mk)
$(call inherit-product-if-exists, external/google-fonts/carrois-gothic-sc/fonts.mk)
$(call inherit-product-if-exists, external/google-fonts/coming-soon/fonts.mk)
$(call inherit-product-if-exists, external/google-fonts/cutive-mono/fonts.mk)
$(call inherit-product-if-exists, external/noto-fonts/fonts.mk)
$(call inherit-product-if-exists, external/roboto-fonts/fonts.mk)
$(call inherit-product-if-exists, external/hyphenation-patterns/patterns.mk)
$(call inherit-product-if-exists, frameworks/base/data/keyboards/keyboards.mk)
$(call inherit-product-if-exists, frameworks/webview/chromium/chromium.mk)
$(call inherit-product-if-exists, $(SRC_TARGET_DIR)/product/core_minimal.mk)
