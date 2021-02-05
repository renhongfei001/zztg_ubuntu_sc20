LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE            := libmofumo

LOCAL_MODULE_TAGS       := optional
LOCAL_C_INCLUDES        += $(TOP)/external/curl/include
LOCAL_C_INCLUDES        += $(TOP)/system/core/include
LOCAL_C_INCLUDES        += $(TOP)/external/libomamo/fumo/_include
LOCAL_C_INCLUDES        += $(TOP)/external/libomamo/fumo/_utils/src

LOCAL_SHARED_LIBRARIES := 
LOCAL_LDLIBS := -llog

IDEV_SRC_FILES := \
					src/fumo.c \
					src/fumo_node.c \
					src/fumo_state_storage.c \
					src/fumo_work_threads.c \
					src/json.c
					
LOCAL_SRC_FILES = $(IDEV_SRC_FILES)
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/verizon/dmclient/lib/

include $(BUILD_SHARED_LIBRARY)
