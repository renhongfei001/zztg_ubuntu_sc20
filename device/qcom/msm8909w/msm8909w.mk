$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)
$(call inherit-product, device/qcom/msm8909/msm8909.mk)

#QTIC flag
-include $(QCPATH)/common/config/qtic-config.mk

TARGET_USES_QCOM_BSP := false
TARGET_USES_NQ_NFC := false

$(call inherit-product, device/google/clockwork/build/clockwork_google.mk)
# include clockwork-services, if present (for full-source PDK builds)
$(call inherit-product-if-exists,vendor/google_clockwork/products/clockwork_services.mk)

PRODUCT_NAME := msm8909w
PRODUCT_DEVICE := msm8909w
PRODUCT_BRAND := ANDROID

# Supported languages for Android Wear
PRODUCT_LOCALES := en_US en_GB en_XA de_DE es_ES es_US fr_FR fr_CA it_IT ja_JP ko_KR pt_BR ru_RU

# System tools
PRODUCT_PACKAGES += \
    applypatch \
    e2fsck \
    libdl \
    libgabi++ \
    local_time.default \
    make_ext4fs \
    power.default \
    run-as \
    setup_fs

PRODUCT_PACKAGES += \
    ethernet-service

PRODUCT_SYSTEM_SERVER_JARS += \
    ethernet-service

PRODUCT_PACKAGES += ClockworkExampleWatchFace

PRODUCT_CHARACTERISTICS := nosdcard,watch

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/wearable_core_hardware.xml:system/etc/permissions/wearable_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.bt.bdaddr_path=/persist/bdaddr.txt

$(call inherit-product, device/google/clockwork/build/wearable-mdpi-512-dalvik-heap.mk)
