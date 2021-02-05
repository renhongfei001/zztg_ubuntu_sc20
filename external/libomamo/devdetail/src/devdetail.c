/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dm_logger.h"
#include "mo_omadmtree.h"
#include "mo_error.h"
#include "pal.h"
#include "plugin_utils.h"

#define PRV_BASE_URI "./DevDetail"
#define PRV_URN      "urn:oma:mo:oma-dm-devdetail:1.0"
#define PRV_BASE_ACL "Get=*"

static void* palHandle = NULL;

static plugin_tree_node_t gNodes[] =
{

    {PRV_BASE_URI,PRV_URN,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "",NULL, NULL, NULL},
    {PRV_BASE_URI"/URI", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "",NULL, NULL, NULL},
    {PRV_BASE_URI"/URI/MaxDepth", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "12",NULL, NULL, NULL},
    {PRV_BASE_URI"/URI/MaxTotLen", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "127",NULL, NULL, NULL},
    {PRV_BASE_URI"/URI/MaxSegLen", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "32",NULL, NULL, NULL},
    {PRV_BASE_URI"/OEM", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
        "Get=*", "","pal_system_oem_get", NULL, NULL},
    {PRV_BASE_URI"/DevTyp",NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_devtype_get", NULL, NULL},
    {PRV_BASE_URI"/FwV",NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "1.0.0", "pal_system_fwv_get", NULL, NULL},
    {PRV_BASE_URI"/SwV", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_swv_get", NULL, NULL},
    {PRV_BASE_URI"/HwV", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_hwv_get", NULL, NULL},
    {PRV_BASE_URI"/LrgObj", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_support_lrgobj_get",NULL, NULL},
    {PRV_BASE_URI"/Ext", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Ext/DateTime", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Ext/DateTime/Date", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_date_get", NULL, NULL},
    {PRV_BASE_URI"/Ext/DateTime/TimeUTC", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_timeutc_get", NULL, NULL},
    {PRV_BASE_URI"/Ext/HostDevice", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Ext/HostDevice/Manu", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_hostdevice_manu_get", NULL, NULL},
    {PRV_BASE_URI"/Ext/HostDevice/Model", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_hostdevice_model_get", NULL, NULL},
    {PRV_BASE_URI"/Ext/HostDevice/SwV", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_hostdevice_swv_get", NULL, NULL},
    {PRV_BASE_URI"/Ext/HostDevice/FwV", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_hostdevice_fwv_get", NULL, NULL},
    {PRV_BASE_URI"/Ext/HostDevice/HwV", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_hostdevice_hwv_get", NULL, NULL},
    {PRV_BASE_URI"/Ext/HostDevice/DateStamp", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_hostdevice_datestamp_get", NULL, NULL},
    {PRV_BASE_URI"/Ext/HostDevice/DeviceID", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_hostdevice_deviceid_get", NULL, NULL},

    {"NULL", NULL,NULL, OMADM_NOT_EXIST, NULL, NULL,NULL,NULL,NULL},
};

static int prv_init_fn(void **oData)
{
    palHandle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);

    if (!palHandle){
        DM_LOGE("palHandle not initialised %s",dlerror());
        return MO_ERROR_COMMAND_FAILED;
        /**
         * \todo !!! function description and actual behaviour MUST be consistent.
         * according to "omadmtree_mo.h" this function shall return:
         * "SyncML error code"
         * but now we a returning something different.
         */
    }
    *oData = gNodes;
    return MO_ERROR_NONE;
}

static void prv_close_fn(void *iData)
{
    (void)iData;

    if (palHandle) {
        // Not sure if we can do anything with error code provided by dlclose(),
        // so just print error and exit.
        int res = dlclose(palHandle);
        palHandle = NULL;
        if (0 != res)
            DM_LOGE("%s: palHandle not closed %s", __FILE__, dlerror());
    }
}

static int prv_get_fn(dmtree_node_t * nodeP,
                     void * iData)
{
    int i = 0;
    int err = MO_ERROR_COMMAND_FAILED;
    int value;
    int (*getLeafFunc)(data_buffer_t *);

    if (!palHandle){
        /// \todo USE PROPER LOGGER HERE
        DM_LOGE("ERROR! PAL isn't loaded");
        return MO_ERROR_COMMAND_FAILED; /// seems that we can't do anything if PAL not loaded
    }

    data_buffer_t *buffer = malloc(sizeof(data_buffer_t));

    if(buffer == NULL)
        return MO_ERROR_DEVICE_FULL;
    memset(buffer, 0, sizeof(data_buffer_t));
    buffer->size = MAX_BUFFER_SIZE;
    buffer->data = (char*) malloc(sizeof(char) * MAX_BUFFER_SIZE);
    if (buffer->data == NULL){
        if(buffer) {
            free(buffer);
            buffer = NULL;
        }
        return MO_ERROR_DEVICE_FULL;
    }
    memset(buffer->data, 0, sizeof(char) * MAX_BUFFER_SIZE);
    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;

    if(nodeP && nodes) {
        while(nodes[i].uri) {
            if(!strcmp(nodeP->uri, nodes[i].uri)) {
                if(palHandle && nodes[i].getLeafFuncName) {
                    getLeafFunc = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc) {
                        value = getLeafFunc(buffer);
                        DM_LOGI("##get data status <%d>",value);
                        if (value == MO_ERROR_NONE) {

                            free(nodeP->data_buffer);
                            nodeP->data_buffer = strdup(buffer->data);
                            DM_LOGI("##get data <%s>",buffer->data);
                            if (nodeP->data_buffer) {
                                nodeP->data_size = strlen(buffer->data);
                                free(nodeP->format);
                                nodeP->format = strdup(nodes[i].format);
                                if(nodeP->format != NULL){
                                    if (nodeP->type) free(nodeP->type);
                                        nodeP->type = strdup(nodes[i].type);
                                        if(!nodeP->type) {
                                            err = MO_ERROR_DEVICE_FULL;
                                        } else err = MO_ERROR_NONE;
                                        break;
                                } else {
                                    err = MO_ERROR_DEVICE_FULL;
                                    break;
                                }
                            }
                        } else {
                            switch(value){
                            case RESULT_MEMORY_ERROR:
                                err =  MO_ERROR_DEVICE_FULL;
                                break;
                            case RESULT_ERROR_INVALID_STATE:
                                err = MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
                                break;
                            default:err = MO_ERROR_COMMAND_FAILED;
                            }
                            break;
                        }
                    } else {
                        err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                        break;
                    }
                } else {
                    char * child_list = NULL;
                    child_list = get_child_list(nodes, nodeP->uri);
                    free(nodeP->data_buffer);
                    if (child_list) {
                        nodeP->data_buffer = strdup(child_list);
                    } else
                        nodeP->data_buffer = strdup(nodes[i].value);

                    if (nodeP->data_buffer) {
                        nodeP->data_size = strlen(nodeP->data_buffer);
                        free(nodeP->format);
                        nodeP->format = strdup(nodes[i].format);
                        if(nodeP->format){
                            if (nodeP->type) free(nodeP->type);
                                nodeP->type = strdup(nodes[i].type);
                                if(!nodeP->type) {
                                    err = MO_ERROR_DEVICE_FULL;
                                } else err = MO_ERROR_NONE;
                            break;
                        } else {
                            err = MO_ERROR_DEVICE_FULL;
                            break;
                        }
                    }
                }
            }
            i++;
            if((nodes+i) == NULL || nodes[i].uri == NULL){
                break;
            }
        }
    }
    free_buffer(buffer);
    return err;
}


omadm_mo_interface_t * omadm_get_mo_interface()
{
    omadm_mo_interface_t *retVal = NULL;

    retVal = malloc(sizeof(*retVal));
    if (retVal) {
        memset(retVal, 0, sizeof(*retVal));
        retVal->base_uri = strdup(PRV_BASE_URI);
        retVal->isNodeFunc = prv_mo_is_node;
        retVal->initFunc = prv_init_fn;
        retVal->closeFunc = prv_close_fn;
        retVal->findURNFunc = prv_find_urn;
        retVal->getFunc = prv_get_fn;
        retVal->getACLFunc = prv_get_acl_fn;
    }

    return retVal;
}
