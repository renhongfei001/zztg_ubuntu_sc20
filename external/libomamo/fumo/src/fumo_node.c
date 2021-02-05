/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <dlfcn.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "fumo.h"

#define FUMO_MINIMUM_REQUIRED_MEMORY        ((unsigned long int)52428800)

/* forward declaration */
extern int check_avail_space(unsigned long int);

/**
    @param [in, out] g_fumo_work_thread_id, thread ID of
        fumo_thread_Download
*/
pthread_t g_fumo_work_thread_id = 0;
/**
    param [in, out] g_fumo_static_nodes, description of static nodes
*/
fumo_static_nodes g_fumo_static_nodes = {
    { // nodes
        { // NodeRoot
            FUMO_BASE_URI, FUMO_URN,
            OMADM_NODE_IS_INTERIOR, efnf_Node,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET
        },
        { // PkgName
            FUMO_BASE_URI "/" FUMO_URI_PKGNAME, NULL,
            OMADM_NODE_IS_LEAF, efnf_Char,
            { FUMO_URI_PKGNAME "/" \
              FUMO_URI_PKGVERSION "/" \
              FUMO_URI_DOWNLOAD "/" \
              FUMO_URI_UPDATE "/" \
              FUMO_URI_DOWNLOAD_AND_UPDATE "/" \
              FUMO_URI_EXT "/" \
              FUMO_URI_STATE,
              sizeof(
              FUMO_URI_PKGNAME "/" \
              FUMO_URI_PKGVERSION "/" \
              FUMO_URI_DOWNLOAD "/" \
              FUMO_URI_UPDATE "/" \
              FUMO_URI_DOWNLOAD_AND_UPDATE "/" \
              FUMO_URI_EXT "/" \
              FUMO_URI_STATE)-1 },
            FUMO_NODE_ABILITY_GET
        },
        { // PkgVersion
            FUMO_BASE_URI "/" FUMO_URI_PKGVERSION, NULL,
            OMADM_NODE_IS_LEAF, efnf_Char,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET
        },
        { // Download
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD, NULL,
            OMADM_NODE_IS_INTERIOR, efnf_Node,
            { FUMO_URI_PKGURL, sizeof(FUMO_URI_PKGURL)-1 },
            FUMO_NODE_ABILITY_GET | FUMO_NODE_ABILITY_EXEC
        },
        { // DownloadPkgURL
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_PKGURL, NULL,
            OMADM_NODE_IS_LEAF, efnf_Char,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET | FUMO_NODE_ABILITY_REPLACE
        },
        { // DownloadCancel
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_CANCEL, NULL,
            OMADM_NODE_IS_LEAF, efnf_Char,
            { NULL, 0 },
            FUMO_NODE_ABILITY_EXEC
        },
        { // Update
            FUMO_BASE_URI "/" FUMO_URI_UPDATE, NULL,
            OMADM_NODE_IS_INTERIOR, efnf_Node,
            { FUMO_URI_PKGDATA, sizeof(FUMO_URI_PKGDATA)-1 },
            FUMO_NODE_ABILITY_EXEC
        },
        { // UpdatePkgData
            FUMO_BASE_URI "/" FUMO_URI_UPDATE_PKGDATA, NULL,
            OMADM_NODE_IS_LEAF, efnf_Bin,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET
        },
        { // UpdateCancel
            FUMO_BASE_URI "/" FUMO_URI_UPDATE_CANCEL, NULL,
            OMADM_NODE_IS_LEAF, efnf_Char,
            { NULL, 0 },
            FUMO_NODE_ABILITY_EXEC
        },
        { // DownloadAndUpdate
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE, NULL,
            OMADM_NODE_IS_INTERIOR, efnf_Node,
            { FUMO_URI_PKGURL, sizeof(FUMO_URI_PKGURL)-1 },
            FUMO_NODE_ABILITY_EXEC
        },
        { // DownloadAndUpdatePkgURL
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL, NULL,
            OMADM_NODE_IS_LEAF, efnf_Char,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET | FUMO_NODE_ABILITY_REPLACE
        },
        { // DownloadAndUpdateCancel
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL, NULL,
            OMADM_NODE_IS_LEAF, efnf_Char,
            { NULL, 0 },
            FUMO_NODE_ABILITY_EXEC
        },
        { // State
            FUMO_BASE_URI "/" FUMO_URI_STATE, NULL,
            OMADM_NODE_IS_LEAF, efnf_Int,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET
        },
        { // Cancel
            FUMO_BASE_URI "/" FUMO_URI_CANCEL, NULL,
            OMADM_NODE_IS_LEAF, efnf_Char,
            { NULL, 0 },
            FUMO_NODE_ABILITY_EXEC
        },
        { // Ext
            FUMO_BASE_URI "/" FUMO_URI_EXT, NULL,
            OMADM_NODE_IS_INTERIOR, efnf_Node,
            { "Severity/NI", sizeof("Severity/NI")-1 },
            FUMO_NODE_ABILITY_GET
        },
        { // ExtSeverity
            FUMO_BASE_URI "/" FUMO_URI_EXT_SEVERITY, NULL,
            OMADM_NODE_IS_LEAF, efnf_Int,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET | FUMO_NODE_ABILITY_REPLACE
        },
        { // ExtNIWifiOnly
            FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY, NULL,
            OMADM_NODE_IS_LEAF, efnf_Bool,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET | FUMO_NODE_ABILITY_REPLACE
        },
        { // ExtNIWifiOnlyTimer
            FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLYTIMER, NULL,
            OMADM_NODE_IS_LEAF, efnf_Int,
            { NULL, 0 },
            FUMO_NODE_ABILITY_GET | FUMO_NODE_ABILITY_REPLACE
        },
        {
            NULL, NULL,
            OMADM_NODE_NOT_EXIST, efnf_Unknown,
            { NULL, 0 },
            FUMO_NODE_ABILITY_NONE
        }
    } // nodes_
}; // static g_fumo_static_nodes

/**
    @brief findNode, find node by URI in static
    @param [in] uri, URI of searching node
    @param [out] node, found node
    @return MO_ERROR_NONE if success
*/
int find_node(const char *uri, fumo_node **node)
{
    DM_LOGI("FUMO: find_node: uri = %s", (uri? uri:"NULL"));

    if (!node) {
        DM_LOGE("FUMO: find_node: node is NULL");
        return MO_ERROR_COMMAND_FAILED;
    }

    int i=0;
    *node = NULL;

    // search in static nodes
    for(i = 0; i < NODES_COUNT; ++i) {
        if(g_fumo_static_nodes.nodes_[i].uri_ == NULL)
            break;
        if(strcmp(g_fumo_static_nodes.nodes_[i].uri_, uri) == 0) {
            *node = g_fumo_static_nodes.nodes_ + i;
            DM_LOGI("FUMO: find_node: Static nodes index = %d", i);
            return MO_ERROR_NONE;
        }
    }

    DM_LOGE("FUMO: find_node: Node not found");
    return MO_ERROR_NOT_FOUND;
};

/**
    @brief execute "Exec" command
    @param [in] pNode, node for execution
    @param [in] cmdData, parameter past to the EXEC SyncML command
    @param [in] correlator, correlator associated to the EXEC SyncML command
    @return FUMO_RC_ACCEPTED_FOR_PROCESSING if success
*/
int node_command_exec( fumo_node* pNode,
                       const char * cmdData, const char * correlator)
{
    int rc = MO_ERROR_NOT_ALLOWED;
    fumo_thread_args *fta = NULL;

    if(NULL == pNode || NULL == pNode->uri_ || pNode->uri_[0] == '\0') {
        DM_LOGE("FUMO: node_command_exec bad arguments");
        return MO_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("FUMO: node_command_exec g_fumo_work_thread_id=%ld", g_fumo_work_thread_id);

    fumo_storage_set_string_value(offsetof(fumo_storage_t, active_node),
        NAME_MAX, pNode->uri_, strlen(pNode->uri_));

    if( g_fumo_work_thread_id ) {
        /** something is in progress, we can cancel it */
        if(strcmp(pNode->uri_, /** Download/Cancel leaf */
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_CANCEL ) == 0) {
            rc = fumo_download_cancel();
            DM_LOGI("FUMO: FUMO_URI_DOWNLOAD_CANCEL rc=%d", rc);
        }
        else if(strcmp(pNode->uri_, /** Update/Cancel leaf */
            FUMO_BASE_URI "/" FUMO_URI_UPDATE_CANCEL ) == 0) {
            rc = fumo_update_cancel();
            DM_LOGI("FUMO: FUMO_URI_UPDATE_CANCEL, rc=%d", rc);
        }
        /** Cancel, DownloadAndUpdate/Cancel leavs */
        else if(strcmp(pNode->uri_,
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL ) == 0 ||
            strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_CANCEL ) == 0 ) {
            /** @todo another way may be */
            rc = fumo_download_cancel();
            DM_LOGI("FUMO: CANCEL, fumo_download_cancel =%d", rc);
            if(MO_ERROR_NOT_FOUND == rc || MO_ERROR_SUCCESS == rc) {
                rc = fumo_update_cancel();
                DM_LOGI("FUMO: CANCEL, fumo_update_cancel =%d", rc);
            }
        } else {
            if(g_key_deferred == 1) {
                DM_LOGI("FUMO: EXEC - KEY DEFERRED");
                rc = FUMO_RC_PRV_UPDATE_DEFERRED_KEY_DEFERRED;
            } else {
                DM_LOGI("FUMO: EXEC - UPDATE DEFERRED");
                if( (strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD ) == 0)
                    ||(strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_UPDATE ) == 0)
                    ||(strcmp(pNode->uri_,
                             FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE ) == 0)) {
                    int state = 0;
                    rc = fumo_storage_get_state(&state);
                    DM_LOGI("FUMO: fumo state result = %d, [%d]",rc, state);

                    if(state == FUMO_DEVICE_STATE_DOWNLOAD_DEFERRED)
                        rc = FUMO_RC_PRV_DOWNLOAD_DEFERRED;
                    else if (state == FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED)
                        rc = FUMO_RC_PRV_UPDATE_DEFERRED_WIFI_DEFERRED;
                    else {
                        rc = FUMO_RC_PRV_UPDATE_DEFERRED;
                        unsigned long int minimum_required_memory = FUMO_MINIMUM_REQUIRED_MEMORY;
                        /** Check available memory */
                        if (check_avail_space(minimum_required_memory) != MO_ERROR_SUCCESS) {
                            DM_LOGE("FUMO: node_command_exec: memory is low, send status code 403 - LOW_MEMORY to DM server");
                            rc = FUMO_RC_PRV_UPDATE_DEFERRED_LOW_MEMORY;
                        }
                    }
                } else
                    rc = FUMO_RC_UPDATE_DEFERRED;
            }
            DM_LOGI("FUMO: node_command_exec operation in progress");
        }
    } else {
        /** we can start working */
        if(NULL == (fta=(fumo_thread_args*)malloc(sizeof(fumo_thread_args)))) {
            DM_LOGE("FUMO: node_command_exec: Out of memory");
            return MO_ERROR_DEVICE_FULL;
        }
        fta->node_ = pNode;
        fta->cmd_data_ = (cmdData==NULL ? NULL : strdup(cmdData));
        fta->correlator_ = (correlator==NULL ? NULL : strdup(correlator));

        fumo_storage_set_string_value(offsetof(fumo_storage_t, command_data),
            NAME_MAX, cmdData, (cmdData==NULL ? 0 : strlen(cmdData)));
        fumo_storage_set_string_value(offsetof(fumo_storage_t, correlator),
            NAME_MAX, correlator, (correlator==NULL ? 0 : strlen(correlator)));

        if(strcmp(pNode->uri_, /** Download node */
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD ) == 0) {
            fta->event_ = fte_Download;
        }
        else if(strcmp(pNode->uri_, /** Update node */
            FUMO_BASE_URI "/" FUMO_URI_UPDATE ) == 0) {
            fta->event_ = fte_Update;
        }
        else if(strcmp(pNode->uri_, /** DownloadAndUpdate node */
            FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE ) == 0) {
            fta->event_ = fte_DownloadAndUpdate;
        }
        else if(strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_CANCEL) == 0 ||
                strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_UPDATE_CANCEL ) == 0 ||
                strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL ) == 0 ||
                strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_CANCEL ) == 0 ) {
            /** Nothing cancel, no firmware upgrade to terminate */
            DM_LOGI("FUMO: %s nothing cancel", pNode->uri_);
            fta->event_ = fte_Cancel;
        }
        else {
            free(fta->cmd_data_);
            free(fta->correlator_);
            free(fta);
            return MO_ERROR_NOT_ALLOWED;
        }

        /** Check available memory*/
        if((strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_UPDATE ) == 0) ||
           (strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD ) == 0) ||
           (strcmp(pNode->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE ) == 0)) {
            unsigned long int minimum_required_memory = FUMO_MINIMUM_REQUIRED_MEMORY;
            if (check_avail_space(minimum_required_memory) != MO_ERROR_SUCCESS) {
                DM_LOGE("FUMO: node_command_exec: memory is low, send status code 403 - LOW_MEMORY to DM server");
                rc = FUMO_RC_PRV_UPDATE_DEFERRED_LOW_MEMORY;
            }
        }

        /** check  battery level*/
        int battery_status = 0;
        if ((get_battery_level(&battery_status) == RESULT_SUCCESS) &&
                                                (battery_status == 0)) {
            DM_LOGI("FUMO: battery is low, send status code 403 to DM server");
            rc = FUMO_RC_PRV_UPDATE_DEFERRED_LOW_BATTERY;
        } else {
            g_key_deferred = 0;
            rc = FUMO_RC_ACCEPTED_FOR_PROCESSING;
        }

        if(pthread_create(&g_fumo_work_thread_id,
                    NULL, fumo_work_thread, fta)) {
            free(fta->cmd_data_);
            free(fta->correlator_);
            free(fta);
            DM_LOGE("FUMO: node_command_exec: Work thread starting failed");
            return MO_ERROR_COMMAND_FAILED;
        }

    }
    return rc;
};

/**
    @brief convering fumo_nodeFormat to string of format
    @param [in] nf, node's format as type
    @param [out] format, node's format, shall be freeded by "free"
    @return !0 if memory isn't enough
*/
int get_string_node_format(fumo_node_format nf, char **format)
{
    if (!format) {
        DM_LOGE("FUMO: get_string_node_format: Invalid parameter format=NULL");
        return 1;
    }
    DM_LOGI("FUMO: get_string_node_format: Numeric format = %d", nf);
    *format = NULL;
    switch(nf) {
        case efnf_Node:
            *format = strdup("node");
            break;
        case efnf_Char:
            *format = strdup("chr");
            break;
        case efnf_Bin:
            *format = strdup("bin");
            break;
        case efnf_Int:
            *format = strdup("int");
            break;
        case efnf_Bool:
            *format = strdup("bool");
            break;
        case efnf_Unknown:
        default:
            *format = strdup("unknown");
            break;
    };

    if (*format) {
        DM_LOGI("FUMO: get_string_node_format: String format is %s", *format);
    } else {
        DM_LOGI("FUMO: get_string_node_format: Out of memory");
    }

    return (*format ? 0 : 1);
};

/**
    @brief convering fumo_nodeFormat to string of type
    @param [in] nf, node's format as type
    @param [out] format, node's format, shall be freeded by "free"
    @return !0 if memory isn't enough
*/
int get_string_node_type(fumo_node_format nf, char **type)
{
    DM_LOGI("FUMO: get_string_node_type: Numeric format = %d", nf);
    int rc = 0;
    *type = NULL;

    switch(nf) {
        case efnf_Unknown:
        case efnf_Bin:
        case efnf_Int:
        case efnf_Char:
        case efnf_Bool:
            *type = strdup("text/plain");
            if (*type == NULL) {
                DM_LOGE("FUMO: get_string_node_type: Out of memory");
                rc = 1;
            }
            break;
        case efnf_Node:
        default:
            break;
    };

    if (*type) {
        DM_LOGI("FUMO: get_string_node_type: Node type is %s", *type);
    } else {
        DM_LOGE("FUMO: get_string_node_type: Node type is NULL");
    }

    return rc;
};

/**
    @brief It returns node's value as bytes array, accrording to
            node format
    @param [in] n, node object
    @param [out] size, data size
    @param [out] data, data as bytes array
    @return MO_ERROR_NONE if success
*/
int get_node_value(const fumo_node* n,
                   unsigned int* size, char** data)
{
    int rc = MO_ERROR_COMMAND_FAILED;
    int state = 0;
    void* pal_handle = NULL;
    ft_pal_system_fwv_get pal_system_fwv_get_func = NULL;
    data_buffer_t frmwrvesrion_buffer;
    char frmwrvesrion[NAME_MAX], pack_name[NAME_MAX];
    FILE *fd = NULL;
    int wifi_only = 0;
    int wifi_only_timer = 23;

    if(!n || !size || !data) {
        DM_LOGE("FUMO: get_node_value: Invalid parameters");
        return MO_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("FUMO: get_node_value: Getting value for %s", n->uri_);

    *size = 0;
    *data = NULL;

    /** return package name */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_PKGNAME)) {
        memset(pack_name, 0, sizeof(pack_name));
        if(NULL == (fd=fopen(
            MO_WORK_PATH FUMO_STATE_DIR PAL_FILENAME_FIRMWARE_PACKET_NAME,
            "r"))) {
            DM_LOGE("FUMO: get_node_value: can't open %s%s%s",
                MO_WORK_PATH, FUMO_STATE_DIR,
                PAL_FILENAME_FIRMWARE_PACKET_NAME);
            return MO_ERROR_COMMAND_FAILED;
        }
        fread(pack_name, sizeof(pack_name), 1, fd);
        *data = strdup(pack_name);
        *size = ( *data != NULL ? strlen(pack_name) : 0 );
        fclose(fd);
        return MO_ERROR_NONE;
    }

    /** return package version */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_PKGVERSION)) {
        if(NULL == (pal_handle=dlopen(
                            PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY))) {
            DM_LOGE("FUMO get_node_value: Can't open library: %s",
                PAL_INSTALL_DIR "/" PAL_LIB_NAME);
            return MO_ERROR_COMMAND_FAILED;
        }

        if(NULL == (pal_system_fwv_get_func=dlsym(
                                        pal_handle, "pal_system_fwv_get"))) {
            DM_LOGE("FUMO get_node_value: Can't get pal_system_fwv_get from %s",
                PAL_INSTALL_DIR "/" PAL_LIB_NAME);
            dlclose(pal_handle);
            return MO_ERROR_COMMAND_FAILED;
        }

        memset(&frmwrvesrion_buffer, 0, sizeof(frmwrvesrion_buffer));
        memset(frmwrvesrion, 0, sizeof(frmwrvesrion));
        frmwrvesrion_buffer.data = frmwrvesrion;
        frmwrvesrion_buffer.size = sizeof(frmwrvesrion);
        if(RESULT_SUCCESS !=
            (rc=pal_system_fwv_get_func(&frmwrvesrion_buffer))) {
            DM_LOGE("FUMO get_node_value: pal_system_fwv_get fail, rc=%d", rc);
            dlclose(pal_handle);
            return MO_ERROR_COMMAND_FAILED;
        }
        if(NULL == (*data=(char*)malloc(strlen(frmwrvesrion_buffer.data)))) {
            DM_LOGE("FUMO get_node_value: out of memory for frmwrvesrion_buffer.data");
            dlclose(pal_handle);
            return MO_ERROR_DEVICE_FULL;
        }
        memcpy(*data, frmwrvesrion_buffer.data,
            strlen(frmwrvesrion_buffer.data));

        *size = strlen(frmwrvesrion_buffer.data);
        DM_LOGI("FUMO: get_node_value: Node value=%s, Size= %d", *data, *size);
        dlclose(pal_handle);
        return MO_ERROR_NONE;
    }

    /** return state */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_STATE)) {
        if(NULL == (*data = (char*)malloc(256)))
            return MO_ERROR_DEVICE_FULL;
        if(MO_ERROR_NONE != (rc=fumo_storage_get_state(&state)))
            return rc;
        snprintf(*data, 256, "%d", state);
        *size = strlen(*data);
        /** @todo
        if(NULL == (*data = (int*)malloc(sizeof(int))))
            return MO_ERROR_DEVICE_FULL;
        if(MO_ERROR_NONE != (rc=fumo_storage_get_state(*data)))
            return rc;
        *size = sizeof(int); */
        return MO_ERROR_NONE;
    }

    /** return download package url */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_PKGURL)) {
        if(MO_ERROR_NONE != (rc=fumo_storage_get_string_value(
                offsetof(fumo_storage_t, download_pkg_url),
                PATH_MAX, data))) {
            DM_LOGE("FUMO: get_node_value download_pkg_url rc = %d", rc);
            return rc;
        }
        if(*data != NULL)
            *size = strlen(*data);
        return MO_ERROR_NONE;
    }

    /** return download and update package url */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL)) {
        if(MO_ERROR_NONE != (rc=fumo_storage_get_string_value(
                offsetof(fumo_storage_t, download_update_pkg_url),
                PATH_MAX, data))) {
            DM_LOGE("FUMO: get_node_value download_update_pkg_url rc = %d", rc);
            return rc;
        }
        if(*data != NULL)
            *size = strlen(*data);
        return MO_ERROR_NONE;
    }

    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY)) {
        if(MO_ERROR_NONE != (rc=fumo_storage_get_int_value(
                offsetof(fumo_storage_t, wifi_only), &wifi_only))) {
            DM_LOGE("FUMO: get_node_value wifi_only rc = %d", rc);
            return rc;
        }
        if(wifi_only==0) {
            if(NULL == (*data=strdup("false"))) return MO_ERROR_DEVICE_FULL;
        } else {
            if(NULL == (*data=strdup("true"))) return MO_ERROR_DEVICE_FULL;
        }
        *size = strlen(*data);
        return MO_ERROR_NONE;
    }

    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLYTIMER)) {
        if(MO_ERROR_NONE != (rc=fumo_storage_get_int_value(
                offsetof(fumo_storage_t, wifi_only_timer), &wifi_only_timer))) {
            DM_LOGE("FUMO: get_node_value wifi_only_timer rc = %d", rc);
            return rc;
        }
        if(NULL == (*data=(char*)malloc(256))) return MO_ERROR_DEVICE_FULL;
        snprintf(*data, 256, "%d", wifi_only_timer);
        *size = strlen(*data);
        return MO_ERROR_NONE;
    }

    /** return severity */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_EXT_SEVERITY)) {
        if(MO_ERROR_NONE != (rc=fumo_storage_get_string_value(
                offsetof(fumo_storage_t, severity), NAME_MAX, data))) {
            DM_LOGE("FUMO: get_node_value severity rc = %d", rc);
            return rc;
        }
        DM_LOGI("FUMO: get_node_value severity = %s", *data);
        if(NULL == *data) return MO_ERROR_DEVICE_FULL;
        *size = strlen(*data);
        return MO_ERROR_NONE;
    }

    if(!n->value_.size_ || !n->value_.data_) {
        DM_LOGI("FUMO: get_node_value: Node does not contain value");
        return MO_ERROR_NONE;
    }

    *data = (char*)malloc(n->value_.size_+1);
    if(!*data) {
        DM_LOGE("FUMO: get_node_value: Out of memory");
        return MO_ERROR_DEVICE_FULL;
    }

    memset(*data, 0, n->value_.size_+1);
    bcopy(n->value_.data_, *data, n->value_.size_);
    *size = n->value_.size_;

    DM_LOGI("FUMO: get_node_value: Value = %p, Size = %d", *data, *size);

    return MO_ERROR_NONE;
};

/**
    @brief It sets node's value as bytes array, accrording to
            node format
    @param [in, out] n, node object
    @param [in] size, data size
    @param [in] data, data as bytes array
    @return MO_ERROR_NONE if success
*/
int set_node_value(fumo_node* n,
                   const unsigned int size, const char* data)
{
    int rc = MO_ERROR_COMMAND_FAILED;
    char *tmp_buff = NULL;
    int wifi_only = 0;
    int wifi_only_timer = 23;
    DM_LOGI("FUMO: set_node_value: Node = %p, Size = %d, Data = %p", n, size, data);

    if( g_fumo_work_thread_id ) {
        DM_LOGE("FUMO: set_node_value: another exec command is in progress");
        /** Check available memory */
        unsigned long int minimum_required_memory = FUMO_MINIMUM_REQUIRED_MEMORY;
        if (check_avail_space(minimum_required_memory) != MO_ERROR_SUCCESS) {
            DM_LOGE("FUMO: node_command_exec: memory is low, send status code 403 - LOW_MEMORY to DM server");
            return FUMO_RC_PRV_UPDATE_DEFERRED_LOW_MEMORY;
        } else {
            return FUMO_RC_UPDATE_DEFERRED;
        }
    }

    if(!size || !data) {
        DM_LOGI("FUMO: set_node_value: Clean node value");
         if( ! n->value_.data_ )
             free(n->value_.data_);
         n->value_.size_ = 0;
         n->value_.data_ = NULL;
         return MO_ERROR_NONE;
    }

    /** set download package url */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_PKGURL)) {
        if(MO_ERROR_NONE != (rc=fumo_storage_set_string_value(
                offsetof(fumo_storage_t, download_pkg_url),
                PATH_MAX, data, size))) {
            DM_LOGE("FUMO set_node_value: fumo_storage_set_string_value rc = %d", rc);
            return rc;
        }
        return MO_ERROR_NONE;
    }

    /** set download and update package url */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL)) {
        if(MO_ERROR_NONE != (rc=fumo_storage_set_string_value(
                offsetof(fumo_storage_t, download_update_pkg_url),
                PATH_MAX, data, size))) {
            DM_LOGE("FUMO set_node_value: fumo_storage_set_string_value rc = %d", rc);
            return rc;
        }
        return MO_ERROR_NONE;
    }

    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY)) {
        if(NULL == (tmp_buff=(char*)malloc(256))) return MO_ERROR_DEVICE_FULL;
        memset(tmp_buff, 0, 256);
        memcpy(tmp_buff, data, (size<256 ? size : 255));
        wifi_only = 0;
        if(0 == strcasecmp(tmp_buff, "true")) wifi_only = 1;
        free(tmp_buff);
        if(MO_ERROR_NONE != (rc=fumo_storage_set_int_value(
                offsetof(fumo_storage_t, wifi_only), wifi_only))) {
            DM_LOGE("FUMO: set_node_value wifi_only rc = %d", rc);
            return rc;
        }
        return MO_ERROR_NONE;
    }

    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLYTIMER)) {
        if(NULL == (tmp_buff=(char*)malloc(256))) return MO_ERROR_DEVICE_FULL;
        memset(tmp_buff, 0, 256);
        memcpy(tmp_buff, data, (size<256 ? size : 255));
        wifi_only_timer = atoi(tmp_buff);
        free(tmp_buff);
        if(MO_ERROR_NONE != (rc=fumo_storage_set_int_value(
                offsetof(fumo_storage_t, wifi_only_timer), wifi_only_timer))) {
            DM_LOGE("FUMO: set_node_value wifi_only_timer rc = %d", rc);
            return rc;
        }
        return MO_ERROR_NONE;
    }

    /** set severity */
    if(0 == strcmp(n->uri_, FUMO_BASE_URI "/" FUMO_URI_EXT_SEVERITY)) {
        DM_LOGE("FUMO: set_node_value severity =%s", data);
        if(MO_ERROR_NONE != (rc=fumo_storage_set_string_value(
                offsetof(fumo_storage_t, severity), NAME_MAX, data, size))) {
            DM_LOGE("FUMO: set_node_value severity rc = %d", rc);
            return rc;
        }
        return MO_ERROR_NONE;
    }

    char *newArray = (char*)realloc(n->value_.data_, size);
    if(!newArray) {
        DM_LOGE("FUMO: set_node_value: Out of memory");
         return MO_ERROR_DEVICE_FULL;
    }

    bcopy(data, newArray, size);
    n->value_.data_ = newArray;
    n->value_.size_ = size;

    DM_LOGI("FUMO: set_node_value: NewData = %p, NewSize = %d", newArray, size);

    return MO_ERROR_NONE;
};


int fumo_continue_job(void * data)
{
    char *active_node = NULL;
    int state = FUMO_DEVICE_STATE_IDLE;
    int rc = MO_ERROR_NONE;
    char *command_data = NULL;
    char *correlator = NULL;

    fumo_storage_get_state(&state);
    DM_LOGI("FUMO fumo_continue_job: state = %d",state);
    /** states list
     *  FUMO_DEVICE_STATE_IDLE
     *  FUMO_DEVICE_STATE_DOWNLOAD_FAILED
     *  FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING
     *  FUMO_DEVICE_STATE_DOWNLOAD_COMPLETE
     *  FUMO_DEVICE_STATE_READY_TO_UPDATE
     *  FUMO_DEVICE_STATE_UPDATE_PROGRESSING
     *  FUMO_DEVICE_STATE_UPDATE_FAILED
     *  FUMO_DEVICE_STATE_UPDATE_FAILED_NO_DATA
     *  FUMO_DEVICE_STATE_UPDATE_COMPLETE_DATA
     *  FUMO_DEVICE_STATE_UPDATE_COMPLETE_NO_DATA
     */
    if( state == FUMO_DEVICE_STATE_IDLE ||
        state == FUMO_DEVICE_STATE_UPDATE_FAILED ||
        state == FUMO_DEVICE_STATE_UPDATE_FAILED_NO_DATA ||
        state == FUMO_DEVICE_STATE_UPDATE_COMPLETE_DATA ||
        state == FUMO_DEVICE_STATE_UPDATE_COMPLETE_NO_DATA) {
            DM_LOGI("FUMO fumo_continue_job: clean up");
            fumo_cleanup();
            remove_network_up_flag();
            return MO_ERROR_NONE;
    }

    fumo_storage_get_string_value(offsetof(fumo_storage_t, active_node),
        NAME_MAX, &active_node);
    fumo_storage_get_string_value(offsetof(fumo_storage_t, command_data),
        NAME_MAX, &command_data);
    fumo_storage_get_string_value(offsetof(fumo_storage_t, correlator),
        NAME_MAX, &correlator);
    rc = fumo_root_exec_fn(active_node, command_data, correlator, data);
    DM_LOGI("FUMO fumo_continue_job: active_node=%s", active_node);
    DM_LOGI("FUMO fumo_continue_job: command_data=%s", command_data);
    DM_LOGI("FUMO fumo_continue_job: correlator=%s", correlator);
    DM_LOGI("FUMO fumo_continue_job: fumo_root_exec_fn rc=%d", rc);

    if(rc == MO_ERROR_SUCCESS || rc == FUMO_RC_ACCEPTED_FOR_PROCESSING)
        rc = MO_ERROR_NONE;
    return rc;
};
