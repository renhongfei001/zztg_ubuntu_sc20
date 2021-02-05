LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
 
LOCAL_MODULE    := libmodcmo

##LOCAL_C_INCLUDES := $(LOCAL_PATH)/src 
LOCAL_C_INCLUDES :=$(LOCAL_PATH)  $(LOCAL_PATH)/../_common/ \
$(LOCAL_PATH)/../../libdmclient/_include/ \
$(LOCAL_PATH)/../../libdmclient/_utils/src/ \
$(LOCAL_PATH)/../../libdmclient/omadm/external/

LOCAL_SRC_FILES :=  \
src/dcmo.c \
        ../_common/plugin_utils.c

LOCAL_SHARED_LIBRARIES := \
    libdl

LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/verizon/dmclient/lib/
LOCAL_CFLAGS := -DCURRENT_DATA_LOCATION=\"/data/vendor/verizon/dmclient/data/\" \
				-DPAL_INSTALL_DIR=\"/system/lib/\" \
				-DPAL_LIB_NAME=\"libpal.so\" \
				-DMO_INSTALL_DIR=\"/system/vendor/verizon/dmclient/lib/\" \
				-DINIT_DATA_LOCATION=\"/data/vendor/verizon/dmclient/data/\" \
				-DDMC_PLATFORM=\"mn6\" \
				-DCURRENT_DATA_FILE_NAME=\"dmacc-current.txt\"

include $(BUILD_SHARED_LIBRARY)

