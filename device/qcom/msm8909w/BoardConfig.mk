# Copyright (C) 2011 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# config.mk
#
# Product-specific compile-time definitions.
#

include device/qcom/msm8909/BoardConfig.mk

TARGET_KERNEL_CROSS_COMPILE_PREFIX := ${ANDROID_BUILD_TOP}/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-

USE_CAMERA_STUB := false

TARGET_USES_AOSP := false

TARGET_USE_QCOM_BIONIC_OPTIMIZATION := true
TARGET_USE_KINGFISHER_OPTIMIZATION := true

#Set sepolicy to run in permissive mode
BOARD_KERNEL_CMDLINE := console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=qcom msm_rtb.filter=0x237 ehci-hcd.park=3 lpm_levels.sleep_disabled=1 earlyprintk androidboot.selinux=permissive

# Set the framebuffer size to 2 for low ram targets
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3
TARGET_ARCH_LOWMEM := true
TARGET_USES_IMS := false
#add suffix variable to uniquely identify the board
#TARGET_BOARD_SUFFIX := _512

HAVE_SYNAPTICS_I2C_RMI4_FW_UPGRADE := true

#Disable below Audio-features for wearable variant
BOARD_SUPPORTS_SOUND_TRIGGER := false

#Use dlmalloc instead of jemalloc for mallocs
MALLOC_IMPL := dlmalloc

#Enables some memory savings.
TARGET_LEAN := true

#Disable NFC
TARGET_USES_QCA_NFC := false
TARGET_ENABLE_SMARTCARD_SERVICE := false

#Disable SVA listen app
AUDIO_FEATURE_ENABLED_LISTEN := false
BOARD_SUPPORTS_SOUND_TRIGGER := false

# Added to indicate that protobuf-c is supported in this build
PROTOBUF_SUPPORTED := true

# Flag to differentiate WEAR builds
TARGET_SUPPORTS_ANDROID_WEAR := true

BOARD_HAVE_BLUETOOTH := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := \
  device/google/clockwork/include/bluetooth \
  device/qcom/common
