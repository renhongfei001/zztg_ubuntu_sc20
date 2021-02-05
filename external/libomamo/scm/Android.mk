LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
 
LOCAL_MODULE    := libmoscm

##LOCAL_C_INCLUDES := $(LOCAL_PATH)/src 
LOCAL_C_INCLUDES :=$(LOCAL_PATH)  $(LOCAL_PATH)/../_common/ \
$(LOCAL_PATH)/../../libdmclient/_include/ \
$(LOCAL_PATH)/../../libdmclient/_utils/src/ \
$(LOCAL_PATH)/../../libdmclient/omadm/external/

LOCAL_SRC_FILES :=  src/scm.c \
src/scm_download.c  \
src/scm_inactive.c  \
src/scm_install.c  \
src/scm_job.c  \
src/scm_mock.c \
src/scm_data.c  \
../_common/plugin_utils.c 

LOCAL_SHARED_LIBRARIES := \
    libdl

LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/verizon/dmclient/lib/
LOCAL_CFLAGS := -DCURRENT_DATA_LOCATION=\"/data/vendor/verizon/dmclient/data/\" \
				-DPAL_INSTALL_DIR=\"/system/lib\" \
				-DPAL_LIB_NAME=\"libpal.so\" \
				-DMO_INSTALL_DIR=\"/system/vendor/verizon/dmclient/lib/\" \
				-DINIT_DATA_LOCATION=\"/data/vendor/verizon/dmclient/data/\" \
				-DDMC_PLATFORM=\"mn6\" \
				-DCURRENT_DATA_FILE_NAME=\"dmacc-current.txt\" \
				-DSQLITE_LIB_DIR=\"/system/lib\" \
				-DDATA_BASE_LOCALION=\"/data/vendor/verizon/dmclient/data/\" \
				-DSQLITE_LIB_NAME=\"libsqlite.so\" \
				-DDATA_BASE_NAME=\"scm_nodes.db\"

include $(BUILD_SHARED_LIBRARY)

