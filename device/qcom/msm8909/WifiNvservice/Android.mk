LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#Make sure module can be compiled in all version
LOCAL_MODULE_TAGS :=optional
LOCAL_MODULE    := WifiNvService
LOCAL_SRC_FILES := WifiNvService
LOCAL_MODULE_CLASS := EXECUTABLES

#Compiled as share library file
#include $(BUILD_SHARED_LIBRARY)
#Compiled as executable file
#include $(BUILD_EXECUTABLE)
include $(BUILD_PREBUILT)
