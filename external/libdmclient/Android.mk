LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
 
LOCAL_MODULE    := dmclient

LOCAL_C_INCLUDES := $(LOCAL_PATH)/core/src \
					$(LOCAL_PATH)/_utils/src \
					$(LOCAL_PATH)/_include \
					$(LOCAL_PATH)/omadm/external \
					$(LOCAL_PATH)/omadm/external/dmcore/include \
					$(LOCAL_PATH)/omadm/external/dmcore/src \
					$(LOCAL_PATH)/omadm/external/libmd5-rfc \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src/sml/inc \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src/sml/inc \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src/sml/lib/inc \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src/sml/mgr/inc \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src/sml/wsm/inc \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src/sml/xlt/all \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src/sml/inc/win \
					$(LOCAL_PATH)/omadm/external/SyncMLRTK/src/sml/xlt/inc \
					$(TOP)/hardware/libhardware_legacy/include


LOCAL_SRC_FILES :=  core/src/controller.c \
					core/src/dmc_queue.c \
					core/src/launcher.c \
					core/src/init.c \
					core/src/net_manager.c \
					core/src/plugin_manager.c \
					omadm/src/omadm_controller.c \
					omadm/src/omadm_event_handler.c \
					omadm/src/omadm_interface_handler.c \
					omadm/external/dmcore/src/callbacks.c \
					omadm/external/dmcore/src/codec.c \
					omadm/external/dmcore/src/credentials.c \
					omadm/external/dmcore/src/defaultroot.c \
					omadm/external/dmcore/src/dmtree.c \
					omadm/external/dmcore/src/log.c \
					omadm/external/dmcore/src/momgr.c \
					omadm/external/dmcore/src/omadmclient.c \
					omadm/external/dmcore/src/package0.c \
					omadm/external/dmcore/src/sml2tree.c \
					omadm/external/dmcore/src/uricheck.c \
					omadm/external/dmcore/src/utils.c \
					omadm/external/libmd5-rfc/md5.c \
					omadm/external/SyncMLRTK/src/sml/lib/all/liblock.c \
					omadm/external/SyncMLRTK/src/sml/lib/all/libmem.c \
					omadm/external/SyncMLRTK/src/sml/lib/all/libstr.c \
					omadm/external/SyncMLRTK/src/sml/lib/all/libutil.c \
					omadm/external/SyncMLRTK/src/sml/mgr/all/mgr.c \
					omadm/external/SyncMLRTK/src/sml/mgr/all/mgrcmdbuilder.c \
					omadm/external/SyncMLRTK/src/sml/mgr/all/mgrcmddispatcher.c \
					omadm/external/SyncMLRTK/src/sml/mgr/all/mgrinstancelist.c \
					omadm/external/SyncMLRTK/src/sml/mgr/all/mgrinstancemgr.c \
					omadm/external/SyncMLRTK/src/sml/mgr/all/mgrutil.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltdec.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltdecwbxml.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltdecxml.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltdevinf.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltenc.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltenccom.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltencwbxml.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltencxml.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltmetinf.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xlttags.c \
					omadm/external/SyncMLRTK/src/sml/xlt/all/xltutilstack.c \
					omadm/external/SyncMLRTK/src/sml/lib/all/libutil.c \
					omadm/external/SyncMLRTK/src/sml/lib/all/libutil.c \
					omadm/external/SyncMLRTK/src/sml/lib/all/libutil.c \
					udm/src/udm_controller.c \
					udm/src/udm_interface_handler.c
					
LOCAL_CFLAGS := -DCURRENT_DATA_LOCATION=\"/data/vendor/verizon/dmclient/data\" \
				-DPAL_INSTALL_DIR=\"/system/lib/\" \
				-DPAL_LIB_NAME=\"libpal.so\" \
				-DMO_INSTALL_DIR=\"/system/vendor/verizon/dmclient/lib\" 

LOCAL_SHARED_LIBRARIES += libhardware_legacy

include $(BUILD_EXECUTABLE)
