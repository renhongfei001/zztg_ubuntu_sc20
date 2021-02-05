/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dm_logger.h"
#include "mo_error.h"
#include "mo_omadmtree.h"
#include "pal.h"
#include "plugin_utils.h"

#define PRV_BASE_URI "./DMAcc"
#define PRV_URN      "urn:oma:mo:oma-dm-dmacc:1.0"

static plugin_tree_node_t gNodes[] =
{
    {PRV_BASE_URI,PRV_URN,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "AppID/ServerID/Name/AppAddr/AAuthPref/AppAuth",
         NULL,NULL,NULL},
    {PRV_BASE_URI"/AppID",NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/ServerID",NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/Name",NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAddr",NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         NULL, "Addr/AddrType/Port",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAddr/Addr", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/AppAddr/AddrType", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/AppAddr/Port", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/AAuthPref", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "", NULL, NULL, NULL},
    {PRV_BASE_URI"/AppAuth", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         NULL, "Client/Server",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Client", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData",
         NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Client/AAuthLevel", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Client/AAuthType", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Client/AAuthName", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Client/AAuthSecret", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Replace=*", "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Client/AAuthData", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Replace=*", "", NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Server", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData",
         NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Server/AAuthLevel", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Server/AAuthType", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Server/AAuthName", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         NULL, "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Server/AAuthSecret", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Replace=*", "",NULL,NULL,NULL},
    {PRV_BASE_URI"/AppAuth/Server/AAuthData", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Replace=*", "", NULL, NULL, NULL},
    {"NULL", NULL,NULL, OMADM_NOT_EXIST, NULL, NULL,NULL,NULL,NULL},
};


/**
 * Initializes the object with plugin_tree_node_t struct
 *
 * @param[out] oData Contains plugin_tree_node_t data after initialization
 * @return PLUGIN_SUCCESS
 */
static int prv_init_fn(void **oData)
{
    FILE *file = NULL;
    char *way = INIT_DATA_LOCATION;
    char *cur_way = CURRENT_DATA_LOCATION CURRENT_DATA_FILE_NAME;
    int error_code = MO_ERROR_COMMAND_FAILED;
    //If android
    if (strstr(DMC_PLATFORM, "mn6")){
        check_path(CURRENT_DATA_LOCATION);
    }

	DM_LOGI("DMACC prv_init_fn was used\n");
    if ((file = fopen(cur_way,"r")) == NULL){
        copy_init_to_current(way, cur_way);
    }

    if(file != NULL){
        fclose(file);
    }

    if ( read_from_file(gNodes, cur_way) == PLUGIN_SUCCESS ){
        *oData = gNodes;
        error_code = MO_ERROR_NONE;
    }


    return error_code;
}

/**
 * Sets value for leafs
 *
 * @param[in] nodeP dmtree_node_t node
 * @param[out] iData Data with plugin tree nodes
 * @return \code PLUGIN_SUCCESS \endcode if the node is found for change
 *         \code PLUGIN_NODE_NOT_EXIST \endcode if the node is not exist
 *         \code PLUGIN_ERROR \endcode in another case
 */
static int prv_set_fn(const dmtree_node_t * nodeP,
                     void * iData)
{
    DM_LOGI("DMAcc prv_set_fn() called");
    char * cur_way = CURRENT_DATA_LOCATION CURRENT_DATA_FILE_NAME;
    int i = 0;
    int err = MO_ERROR_COMMAND_FAILED;
    plugin_tree_node_t * nodes = (plugin_tree_node_t*)iData;


    if(nodeP) {
       i = prv_find_node(nodes, nodeP->uri);
       if (i == -1) {
           err = MO_ERROR_NOT_FOUND;
       }
       else {
           nodes[i].value = strdup(nodeP->data_buffer);
           if(nodes[i].value){
               DM_LOGD( "%s\n",nodes[i].uri);
               DM_LOGD( "%s\n",nodes[i].value);
               set_to_file(nodes,cur_way);
               err = MO_ERROR_NONE;
           }
       }
    }
    return err;
}

/**
 * Gets all children of the interior node
 *
 * This function checks if there is a interior node
 * if so, finds its children
 *
 * @param[in] nodeP dmtree_node_t node
 * @param[out] iData Data with plugin tree nodes
 * @return \code PLUGIN_SUCCESS \endcode if the node is found
 *         \code PLUGIN_NODE_NOT_EXIST \endcode if the node is not exist
 *         \code PLUGIN_ERROR \endcode in another case
 */
int prv_get_fn(dmtree_node_t * nodeP, void *iData)
{
    DM_LOGI("DMAcc prv_get_fn() called");
    int err = MO_ERROR_NONE;

    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;
    int i;

    if(nodeP) {
        nodeP->format = NULL;
        nodeP->type = NULL;
        nodeP->data_size = 0;
        nodeP->data_buffer = NULL;

        i = prv_find_node(nodes, nodeP->uri);
        if (i == -1) {
            err = MO_ERROR_NOT_FOUND;
            goto on_error;
        }

        if (!strcmp(nodes[i].format,OMADM_LEAF_FORMAT_CHAR)) {
            nodeP->format = strdup(nodes[i].format);
            nodeP->type = strdup(nodes[i].type);
            if (!nodeP->format || !nodeP->type) {
                err = MO_ERROR_DEVICE_FULL;
                goto on_error;
            }
        } else {
            nodeP->format = strdup(nodes[i].format);
            if (!nodeP->format) {
                err = MO_ERROR_DEVICE_FULL;
                goto on_error;
            }

        }
        if (strstr(nodes[i].uri, "Client/AAuthName") && !strcmp(nodes[i].value, "")){
            static void* palHandle = NULL;
            int (*get_imei_func)(data_buffer_t *);
            int value;
            data_buffer_t *buffer = (data_buffer_t *) malloc(sizeof(data_buffer_t));

            if (buffer == NULL)
                return MO_ERROR_DEVICE_FULL;
            memset(buffer, 0, sizeof(data_buffer_t));
            buffer->size = MAX_BUFFER_SIZE;
            buffer->data = (char*) malloc(sizeof(char) * MAX_BUFFER_SIZE);
            if (buffer->data == NULL){
                if (buffer){
                    free(buffer);
                    buffer = NULL;
                }
                return MO_ERROR_DEVICE_FULL;
            }

            palHandle = getPalHandle(PAL_INSTALL_DIR "/" PAL_LIB_NAME);

            if (!palHandle){
                DM_LOGD( "%s\n", dlerror());
                if(buffer->data){
                    free(buffer->data);
                    buffer->data = NULL;
                }
                if(buffer){
                    free(buffer);
                    buffer = NULL;
                }
                return MO_ERROR_DEVICE_FULL;
            }
            get_imei_func = dlsym(palHandle, "pal_system_dev_id_get");
                if (get_imei_func) {
                    value = get_imei_func(buffer);
                    DM_LOGI("##get_imei_func <%d>",value);
                    if (value == MO_ERROR_NONE) {
                        free(nodeP->data_buffer);
                        // remove IMEI prefix from devId if exist
                        if( strstr(buffer->data, "IMEI:") ) {
                            char *p = strtok(buffer->data, ":");
                            p = strtok('\0', ":");
                            nodeP->data_buffer = strdup(p);
                        } else {
                            nodeP->data_buffer = strdup(buffer->data);
                        }
                        if(nodeP->data_buffer != NULL){
                            nodeP->data_size = strlen(nodeP->data_buffer);
                            free(buffer->data);
                            free(buffer);
                            err = MO_ERROR_NONE;
                        } else err = MO_ERROR_DEVICE_FULL;
                    } else {
                        err = MO_ERROR_COMMAND_FAILED;
                    }
                }

            return err;
        }

        if (nodes[i].value) {

            char * child_list = NULL;
            child_list = get_child_list(nodes, nodeP->uri);
            if (child_list) {
                nodeP->data_buffer = strdup(child_list);
            } else nodeP->data_buffer = strdup(nodes[i].value);

            if (!nodeP->data_buffer) {
                err = MO_ERROR_DEVICE_FULL;
                goto on_error;
            }
            else nodeP->data_size = strlen(nodes[i].value);
        }

        return err;
    } else {
        return MO_ERROR_COMMAND_FAILED;
    }
on_error:
    DM_LOGE("DMAcc prv_get_fn() ERROR happened err = %d", err);

    if (nodeP->format) free (nodeP->format);
    if (nodeP->type) free (nodeP->type);
    // allocating nodeP->data_buffer is the last thing that can fail so no need to free it.

    return err;
}

/**
 * Set omadm_get_mo_interface function
 *
 * @return retVal Omadm mo interface tree
 */

omadm_mo_interface_t * omadm_get_mo_interface()
{
    omadm_mo_interface_t *retVal = NULL;

    retVal = malloc(sizeof(*retVal));
    if (retVal) {
        memset(retVal, 0, sizeof(*retVal));
        retVal->base_uri = strdup(PRV_BASE_URI);
        retVal->isNodeFunc = prv_mo_is_node;
        retVal->initFunc = prv_init_fn;
        retVal->findURNFunc = prv_find_urn;
        retVal->getFunc = prv_get_fn;
        retVal->setFunc = prv_set_fn;
        retVal->getACLFunc = prv_get_acl_fn;
    }

    return retVal;
}
