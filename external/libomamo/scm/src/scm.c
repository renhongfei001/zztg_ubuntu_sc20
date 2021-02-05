/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <dlfcn.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "plugin_utils.h"
#include "scm.h"


/** @brief flg, !=0 if plugin already initialized */
int g_initialized = 0;

/** @brief flg, !=0 if GUI is supported by PAL */
int gui_supported = 1;

/** @brief flg, !=0 if neworking events are supported */
int network_events = 0;

/** @brief handle for network events in PAL */
void* pal_handle_network_events = NULL;

/// PAL functiom names:
#define PAL_DOWNLOAD_SOFTWERE_FN "pal_download_software"
#define PAL_GUI_CREATE_MSG_FN    "pal_fumo_gui_create_message"
#define PAL_GUI_DESTROY_MSG_FN   "pal_fumo_gui_destroy_message"
#define PAL_SCM_CLOSE_FN         "pal_scm_close"
#define PAL_SCM_INIT_FN          "pal_scm_init"

#define SCM_DOWNLOAD_NODE         "./SCM/download/"
#define SCM_DEPLOYED_NODE         "./SCM/inventory/deployed"
#define SCM_DELIVERED_NODE        "./SCM/inventory/delivered"
#define SCM_PACKAGE_URL           "/PkgURL"
#define SCM_PACKAGE_ID            "/PkgID"
#define SCM_PACKAGE_DATA          "/Data"
#define SCM_PACKAGE_IDREF         "PkgIDRef"
#define SCM_PARENT_URI_DOWNLOAD   "download"
#define SCM_PARENT_URI_DEPLOYED   "deployed"
#define SCM_PARENT_URI_DELIVERED  "delivered"

/*!
 * @brief Callback to initialize the MO (MANDATORY)
 *
 * @param [out] dataP opaque pointer to MO internal data
 *
 * @returns a MO error code
 */
int scm_root_init_fn(void **dataP)
{
    //TODO: return will uncomment when pal function be implemented
    DM_LOGI("SCOMO: scm_root_init_fn");
    (void)dataP;
    void* pal_handle = NULL;
    pal_notify_on_network_conditions_changed_t set_newt_evens_cb = NULL;
    int (*pal_scm_init_fc)(void) = NULL;

    if(g_initialized) {
        DM_LOGI("SCOMO: scm_root_init_fn, already initialized");
        return MO_ERROR_NONE;
    }

    if(NULL == (pal_handle=dlopen(
                        PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY))) {
        DM_LOGE("SCOMO: Can't open library: %s",
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        //return MO_ERROR_COMMAND_FAILED;
    }

    if(NULL == dlsym(pal_handle, PAL_DOWNLOAD_SOFTWERE_FN)) {
        DM_LOGE("SCOMO: Can't get %s from %s",PAL_DOWNLOAD_SOFTWERE_FN,
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        //dlclose(pal_handle);
        //return MO_ERROR_COMMAND_FAILED;
    }
    if(NULL == dlsym(pal_handle, PAL_GUI_CREATE_MSG_FN)) {
        DM_LOGE("SCOMO: Can't get %s from %s", PAL_GUI_CREATE_MSG_FN,
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        gui_supported = 0;
    }

    if(NULL == dlsym(pal_handle, PAL_GUI_DESTROY_MSG_FN)) {
        DM_LOGE("SCOMO: Can't %s from %s", PAL_GUI_DESTROY_MSG_FN,
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        gui_supported = 0;
    }

    if(NULL != dlsym(pal_handle,
            PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES) &&
        NULL != dlsym(pal_handle,
            PAL_NOTIFY_STOP_NETWORK_CONDITIONS_CHANGES)) {
        network_events = 1;
    } else {
        DM_LOGI("SCOMO: Can't get PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES \
        and PAL_NOTIFY_STOP_NETWORK_CONDITIONS_CHANGES from %s",
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
    }

    if(NULL == (pal_scm_init_fc = dlsym(pal_handle, PAL_SCM_INIT_FN)) ||
        0 != pal_scm_init_fc()) {
        DM_LOGI("SCOMO: Can't execute a %s",PAL_SCM_INIT_FN);
        //dlclose(pal_handle);
        //return MO_ERROR_COMMAND_FAILED;
    }

    // Init scm data base
    if( init_lib() != MO_ERROR_NONE || init_database() != MO_ERROR_NONE) {
           DM_LOGE("SCOMO: scm_root_init_fn, can't init database");
           return MO_ERROR_COMMAND_FAILED;
    }

    dlclose(pal_handle);

    g_initialized = 1;

    /** subscribing to network's events */
    if(0 == network_events ||
            NULL == (pal_handle_network_events=dlopen(
                        PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY)) ||
            NULL == (set_newt_evens_cb=dlsym(pal_handle_network_events,
                    PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES)) ||
            0 > set_newt_evens_cb(network_conditions_changed) ) {
        DM_LOGE("SCOMO: Can't subscribe to network events");
    }

    return MO_ERROR_NONE;
}

/*!
 * @brief Callback to free the MO
 *
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 */
void scm_root_close_fn(void *data)
{
    (void) data;
    DM_LOGI("SCOMO: scm_root_close_fn");

    int (*pal_scm_close_fc)(void) = NULL;
    void* pal_handle = NULL;

    if(NULL == (pal_handle=dlopen(
                        PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY)) ||

       NULL == (pal_scm_close_fc=dlsym(pal_handle, PAL_SCM_CLOSE_FN)) ||
                0 != pal_scm_close_fc() ) {
       DM_LOGE("SCOMO: scm_root_close_fn, can't %s",PAL_SCM_CLOSE_FN);
    }

    // TODO close scm database

    if( close_database() != MO_ERROR_NONE) {
        DM_LOGE("SCOMO: scm_root_close_fn, can't close database");
     }

    if(pal_handle) dlclose(pal_handle);
}

/*!
 * @brief Callback to get the type of a node
 *
 * @param [in] uri URL of the node
 * @param [out] type type of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_is_node_fn(const char *uri, omadmtree_node_kind_t *type,
                                void *data)
{
    (void)data;
    int rc = MO_ERROR_NOT_EXECUTED;

    scm_node *node = NULL;

    if(!type) {
        DM_LOGE("SCOMO: scm_root_is_node_fn: Invalid parameter type=NULL");
        return MO_ERROR_COMMAND_FAILED;
    }
    *type = OMADM_NODE_NOT_EXIST;
    if(!uri || *uri == '\0') {
        DM_LOGE("SCOMO: scm_root_is_node_fn: Empty uri");
        return MO_ERROR_NONE;
    }

    // select node from scomo db
    rc = scm_get_node(uri, &node);

    if(rc == MO_ERROR_NONE )
        *type = node->node_kind_;
    else if(rc == MO_ERROR_NOT_FOUND){
        *type = OMADM_NODE_NOT_EXIST;
        rc = MO_ERROR_NONE;
    }
    DM_LOGI("SCOMO: scm_root_is_node_fn: Node type = %d, rc = %d", *type, rc);
    return rc;
}

/*!
 * @brief Callback to find the URLs associated to an URN
 *
 * @param [in] urn URN to find
 * @param [out] urlsP null-terminated array of urls, freed by the caller
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_find_urn_fn (const char * urn, char *** urlsP, void * data)
{
    (void)data;
    int count = 0;
    char **urnArray = NULL;

    DM_LOGI("SCOMO: scm_root_find_urn_fn {");
    if(!urn || *urn == '\0' || !urlsP ) {
        DM_LOGE("SCOMO: scm_root_find_urn_fn: invalid parameters");
        return MO_ERROR_COMMAND_FAILED;
    }

    *urlsP = NULL;

    urnArray = (char**)realloc( *urlsP, sizeof(char*)*(count+1) );

    if(!urnArray) {
        DM_LOGE("SCOMO: scm_root_find_urn_fn: Out of memory");
        return MO_ERROR_DEVICE_FULL;
    }
    if ( strcmp(urn,SCM_URN) != 0 ) {
        DM_LOGE("SCOMO: invalid urn %s",urn);
        return MO_ERROR_NOT_FOUND;
    }
    // only root node
    urnArray[count] = strdup(SCM_BASE_URI);

    if(!urnArray[count]) {
        free(urnArray);
        return MO_ERROR_DEVICE_FULL;
    }
    urnArray[++count] = NULL;
    *urlsP = urnArray;

    return MO_ERROR_NONE;
}


/*!
 * @brief Callback to get the value of a node
 *
 * result is stored in the nodeP parameter. If the targeted node is an
 * interior node, the nodeP->data_buffer must be a char * containing
 * the node's children's names separated by '/'.
 *
 * @param [in, out] nodeP the node to retrieve
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_get_fn (dmtree_node_t * nodeP, void * data)
{
    (void)data;
    DM_LOGI("SCOMO: scm_root_get_fn");
    scm_node *node = NULL;

    int rc = MO_ERROR_NOT_EXECUTED;
    unsigned int i = 0;

    DM_LOGI("SCOMO: scm_root_get_fn {");
    if(!nodeP || !nodeP->uri ) {
        DM_LOGE("SCOMO: scm_root_get_fn: Invalid parameters");
        return MO_ERROR_COMMAND_FAILED;
    }
    DM_LOGI("SCOMO: scm_root_get_fn uri=%s", nodeP->uri);

    rc = scm_get_node(nodeP->uri, &node);

    if(rc != MO_ERROR_NONE) {
        DM_LOGE("SCOMO: scm_root_get_fn: Node not found");
        return MO_ERROR_NOT_FOUND;
    }

    if( ! (node->node_command_ability_ & SCM_NODE_ABILITY_GET) ) {
        DM_LOGE("SCOMO: scm_root_get_fn: Command GET not supported by node");
        return MO_ERROR_FORBIDDEN;
    }

    if(get_string_node_format(node->format_, &nodeP->format) != 0) {
        DM_LOGE("SCOMO: scm_root_get_fn: Out of memory");
        return MO_ERROR_DEVICE_FULL;
    }

    if(get_string_node_type(node->format_, &nodeP->type) != 0) {
        free(nodeP->format);
        nodeP->format = NULL;
        DM_LOGE("SCOMO: scm_root_get_fn: Out of memory");
        return MO_ERROR_DEVICE_FULL;
    }

    if(node->value_.data_ != NULL) {
        nodeP->data_buffer = strdup(node->value_.data_);
        if(nodeP->data_buffer == NULL) {
            if(nodeP->type) {
                free(nodeP->type);
                nodeP->type = NULL;
            }
            if(nodeP->format) {
                free(nodeP->format);
                nodeP->format = NULL;
            }
            nodeP->data_size = 0;
            nodeP->data_buffer = NULL;
            DM_LOGE("SCOMO: scm_root_get_fn: Could not get node value");
        } else {
           nodeP->data_size = node->value_.size_;
           DM_LOGI("SCOMO: scm_root_get_fn: data-size = %d", nodeP->data_size);
           for(i=0; i<nodeP->data_size; ++i) {
               DM_LOGI("[%02X] ", (0xFF&(nodeP->data_buffer[i])));
           }
        }
    }

    DM_LOGI("SCOMO_TEST: node = %p", node);
    return MO_ERROR_NONE;
}

/*!
 * @brief Callback to set the value of a node
 *
 * The targeted node can already exist. This is used both for ADD
 * and REPLACE MO commands.
 * If nodeP-> type is "node", an interior node must be created.
 *
 * @param [in] nodeP the node to store
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_set_fn (const dmtree_node_t * nodeP, void * data)
{
    (void)data;
    scm_node *node = NULL;
    int rc = MO_ERROR_NOT_EXECUTED;

    DM_LOGI("SCOMO: scm_root_set_fn");
    if(!nodeP || !nodeP->uri ) {
        DM_LOGE("SCOMO: scm_root_set_fn: Invalid parameters");
        return MO_ERROR_COMMAND_FAILED;
    }

    rc = scm_get_node(nodeP->uri, &node);
    if(rc != MO_ERROR_NONE) {
         DM_LOGE("SCOMO: scm_root_set_fn: Node not found, try to add");
         // add new node
         node = calloc(1,sizeof(scm_node));
         if(node == NULL)
             return MO_ERROR_DEVICE_FULL;

         rc = get_node_type_from_string( &node->format_,
                                         &node->node_kind_,
                                         nodeP->format);
         if ( rc != MO_ERROR_NONE) {
             DM_LOGI("SCOMO: invalid node type");
             free(node);
             return MO_ERROR_COMMAND_FAILED;
         }
         node->acl_ = strdup("Get=*&Replace=*&Exec=*&Add=*");

         node->node_command_ability_ = SCM_NODE_ABILITY_REPLACE |
                                           SCM_NODE_ABILITY_GET |
                                           SCM_NODE_ABILITY_EXEC;
         if(nodeP->uri)
             node->uri_ = strdup(nodeP->uri);
         else
             node->uri_ =  NULL;
         node->urn_ = NULL;
         if(nodeP->data_buffer)
             node->value_.data_ = strdup(nodeP->data_buffer);
         else
             node->value_.data_ = NULL;
         node->value_.size_ = nodeP->data_size;

         rc = scm_set_node(nodeP->uri,node);
         if(node->value_.data_)
             free(node->value_.data_);
         if(node->uri_)
             free(node->uri_);
         if(node->acl_)
             free(node->acl_);
         free(node);
         return rc;
    }
    if( ! (node->node_command_ability_ & SCM_NODE_ABILITY_REPLACE) ) {
        DM_LOGE("SCOMO: scm_root_get_fn: Command SET not supported by node");
        return MO_ERROR_FORBIDDEN;
    }
    rc = scm_set_node_prop(nodeP->uri, NODE_VALUE, (void*)nodeP->data_buffer);

    DM_LOGI("SCOMO: scm_root_set_fn: set_node_value return code %d", rc);
    return  rc;
}

/*!
 * @brief Callback to get the ACL of a node
 *
 * The ACL string must be allocated by this function. It will be
 * freed by the caller.
 * If the node has no ACL, *aclP must be NULL.
 *
 * @param [in] uri URL of the node
 * @param [out] aclP ACL of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_get_acl_fn (const char * uri, char ** aclP, void * data)
{

    (void)data;
    int rc = MO_ERROR_NOT_EXECUTED;
    scm_node *node = NULL;

    DM_LOGI("SCOMO: scm_root_get_acl_fn");
    if(!uri || *uri == '\0') {
        DM_LOGE("SCOMO: scm_root_get_acl_fn: Invalid uri");
        return MO_ERROR_COMMAND_FAILED;
    }
    if(!aclP) {
        DM_LOGE("SCOMO: scm_root_get_acl_fn: aclP is NULL");
        return MO_ERROR_COMMAND_FAILED;
    }

    *aclP = NULL;

    DM_LOGI("SCOMO: scm_root_get_acl_fn uri=%s", uri);

    rc = scm_get_node(uri, &node);

    if(rc != MO_ERROR_NONE) {
        DM_LOGE("SCOMO: scm_root_get_acl_fn: Node not found");
        return MO_ERROR_NOT_FOUND;
    }

    *aclP=strdup(node->acl_);
    if(*aclP == NULL ) {
        DM_LOGI("SCOMO: sscm_root_get_acl_fn: out of memory");
        return MO_ERROR_DEVICE_FULL;
    }
    return MO_ERROR_NONE;
}

/*!
 * @brief Callback to set the ACL of a node
 *
 * @param [in] uri URL of the node
 * @param [in] acl ACL of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_set_acl_fn(const char * uri, const char *acl, void * data)
{

    (void)data;
    int rc = MO_ERROR_NOT_EXECUTED;
    char *newAcl = NULL;
    scm_node *node = NULL;

    DM_LOGI("SCOMO: scm_root_set_acl_fn {");
    if(!uri || *uri == '\0') {
        DM_LOGE("SCOMO: scm_root_set_acl_fn: Invalid uri");
        return MO_ERROR_COMMAND_FAILED;
    }

    if(!acl) {
        DM_LOGE("SCOMO: scm_root_set_acl_fn: acl is NULL");
        return MO_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("SCOMO: scm_root_set_acl_fn uri=%s, acl=%s", uri, acl);

    rc = scm_get_node(uri, &node);

    if(rc != MO_ERROR_NONE) {
        DM_LOGE("SCOMO: scm_root_set_acl_fn: Node not found");
        return MO_ERROR_NOT_FOUND;
    }

    newAcl = (char*)realloc(node->acl_, strlen(acl)+1);

    if(!newAcl) {
        DM_LOGE("SCOMO: scm_root_set_acl_fn: Out of memory");
        return MO_ERROR_DEVICE_FULL;
    }

    memset(newAcl, 0, strlen(acl)+1);
    strcpy(newAcl, acl);

    DM_LOGI("SCOMO: scm_root_set_acl_fn:  new ACL = %s", newAcl);

    rc = scm_set_node_prop(uri, NODE_ACL, (void*)newAcl);
    return rc;
}

/*!
 * @brief Callback to execute the function associated to a node
 *
 * @param [in] uri URL of the node
 * @param [in] cmdData parameter past to the EXEC MO command
 * @param [in] correlator correlator associated to the EXEC MO command
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_exec_fn (const char * uri, const char * cmdData,
                             const char * correlator, void * data)
{
    (void)data;
    scm_node *node = NULL;
    int rc = MO_ERROR_NOT_EXECUTED;

    DM_LOGI("SCOMO: scm_root_exec_fn {");
    if(!uri || *uri == '\0') {
        DM_LOGE("SCOMO: scm_root_exec_fn: Invalid uri");
        return MO_ERROR_COMMAND_FAILED;
    }
    if(strstr(uri,"SCM") == NULL)
         return MO_ERROR_COMMAND_FAILED;

    DM_LOGI("SCOMO: scm_root_exec_fn uri=%s  %p", uri, node);

    char * pck_name = NULL;
    char * parent_id = NULL;
    char * command = NULL;
    rc = check_exec_pre_condition(uri,&pck_name,&parent_id, &command);

    if(rc == MO_ERROR_NONE) {
        rc = push_job_to_queue(pck_name, parent_id, command,
                               cmdData, correlator);
    }

    if(pck_name)
        free(pck_name);
    if(parent_id)
        free(parent_id);
    if(command)
        free(command);

    if(rc != SCM_ACCEPTED_FOR_PROCESSING )  // If the Exec is executed asynchronously
        DM_LOGE("SCOMO: scm_root_exec_fn: failed with error code %d", rc);

    DM_LOGI("SCOMO: scm_root_exec_fn }");

    return rc;
}

/*!
 * @brief Entry point of the shared lib
 *
 * The returned pointer ust be allocated by this function.
 * The caller will call closeFunc (if any) before freeing the pointer.
 * The caller will also free the uri string inside.
 *
 * @returns a pointer tothe MO interface
 */
omadm_mo_interface_t * omadm_get_mo_interface(void)
{
    omadm_mo_interface_t *interface =
        (omadm_mo_interface_t*) malloc( sizeof(*interface) );
    if( !interface ) {
        DM_LOGE("SCOMO: omadm_get_mo_interface: Out of memory");
        return 0;
    }

    memset(interface, 0, sizeof(*interface));

    interface->base_uri = strdup(SCM_BASE_URI);
    interface->initFunc = scm_root_init_fn;
    interface->closeFunc = scm_root_close_fn;
    interface->isNodeFunc = scm_root_is_node_fn;
    interface->findURNFunc  = scm_root_find_urn_fn;
    interface->getFunc = scm_root_get_fn;
    interface->setFunc = scm_root_set_fn;
    interface->getACLFunc = scm_root_get_acl_fn;
    interface->setACLFunc = scm_root_set_acl_fn;
    interface->execFunc = scm_root_exec_fn;

    DM_LOGI("SCOMO: omadm_get_mo_interface: interface = %p", interface);
    return interface;
};

/**
    @brief Registering event handler into MO plugin.
    @param [in] data, data was got from initFunc result
    @param [in] event_handler, callback, pointer to event handler
        in DM-client
    @return 0 if sucess, errno if fail
*/
int omadm_mo_register_event_handler(void *data,
                                omadm_mo_ft_event_handler event_handler)
{
    /** @todo use it as pointer to dynamic nodes */
    (void)data;

    DM_LOGI("SCM: omadm_mo_register_event_handler {");
    if( g_dmclient_event_handler || event_handler == NULL ) {
        DM_LOGE("SCM: omadm_mo_register_event_handler failed");
        return 1;
    }

    g_dmclient_event_handler = event_handler;

    DM_LOGI("SCM: omadm_mo_register_event_handler }");
    return 0;
}

/**
    @brief Unregistering event handler into MO plugin.
    @param [in] data, data was got from initFunc result
    @param [in] event_handler, callback, pointer to event handler
        in DM-client
    @return 0 if sucess, errno if fail
*/
int omadm_mo_unregister_event_handler(void *data,
                                omadm_mo_ft_event_handler event_handler)
{
    /** @todo use it as pointer to dynamic nodes */
    (void)data;

    DM_LOGI("SCM: omadm_mo_unregister_event_handler {");
    if(g_dmclient_event_handler != event_handler || event_handler == NULL) {
        DM_LOGE("SCM: omadm_mo_unregister_event_handler failed");
        return 1;
    }

    g_dmclient_event_handler = NULL;

    DM_LOGI("SCM: omadm_mo_unregister_event_handler }");
    return 0;
}

/** @brief removes all temporary files and sets state to default */
void scm_cleanup(void)
{
    /** @todo */
    DM_LOGI("SCOMO: scm_cleanup");
}

int network_conditions_changed(network_conditions_t nc)
{
    DM_LOGI("SCOMO: network_conditions_changed, net_feature=%d,enabled=%d",
            (int)nc.net_feature, (int)nc.enabled);

    return MO_ERROR_NONE;
}

int get_string_node_format(scm_node_format nf, char **format)
{
    if (!format) {
        DM_LOGE("SCOMO: get_string_node_format: Invalid parameter format=NULL");
        return 1;
    }
    DM_LOGI("SCOMO: get_string_node_format: Numeric format = %d", nf);
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
        DM_LOGI("SCOMO: get_string_node_format: String format is %s", *format);
    } else {
        DM_LOGI("SCOMO: get_string_node_format: Out of memory");
    }

    return (*format ? 0 : 1);
};

int get_string_node_type(scm_node_format nf, char **type)
{
    DM_LOGI("SCOMO: get_string_node_type: Numeric format = %d", nf);
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
                DM_LOGE("SCOMO: get_string_node_type: Out of memory");
                rc = 1;
            }
            break;
        case efnf_Node:
        default:
            break;
    };

    if (*type) {
        DM_LOGI("SCOMO: get_string_node_type: Node type is %s", *type);
    } else {
        DM_LOGE("SCOMO: get_string_node_type: Node type is NULL");
    }

    return rc;
};


int get_node_type_from_string(scm_node_format *nf, omadmtree_node_kind_t *nk,
                              char *format)
{
    DM_LOGI("SCOMO: get_node_type_from_string <%s>",format ? format : "NULL");
    int rc = MO_ERROR_COMMAND_FAILED;
    if(format == NULL)
        return rc;
    if((strstr(format,OMADM_NODE_FORMAT))) {
        *nf = efnf_Node;
        *nk = OMADM_NODE_IS_INTERIOR;
    }
    else if( strstr(format,OMADM_LEAF_FORMAT_CHAR) ) {
        *nk = OMADM_NODE_IS_LEAF;
        *nf = efnf_Char;
    } else if( strstr(format,OMADM_LEAF_FORMAT_BOOL) ) {
        *nk = OMADM_NODE_IS_LEAF;
        *nf = efnf_Bool;
    } else if( strstr(format,OMADM_LEAF_FORMAT_INT) ) {
        *nk = OMADM_NODE_IS_LEAF;
        *nf = efnf_Int;
    } else if( strstr(format,OMADM_LEAF_FORMAT_FLOAT) ) {
        *nk = OMADM_NODE_IS_LEAF;
        *nf = efnf_Char;
    } else {
        *nf = efnf_Unknown;
        *nk = OMADM_NODE_NOT_EXIST;
    }

    DM_LOGI("SCOMO: get_node_kind_from_string  kind <%d> format <%d>",*nk, *nf);
    return MO_ERROR_NONE;
}

/**
 * Exec for Download subtree pre-condition:
 * Download/<X> is exist
 * At least the following nodes need to be set with an appropriate value:
 * Download/<X>/PkgID
 * Download/<X>/PkgURL
 * at least one Operation sub-node for the Primitive to be executed
 * Download/<X>/Operations
 *
 * Exec for Delivered subtree pre-condition:
 * Delivered/<X> is exist
 * At least the following nodes need to be set with an appropriate value:
 * Delivered /<X>/PkgID
 * Delivered/<X>/Data
 * at least one Operation sub-node for the Primitive to be executed
 * Delivered/<X>/Operations
 *
 * Exec for Deployed subtree pre-condition:
 * Deployed/<X> is exist
 * At least the following nodes need to be set with an appropriate value:
 * Deployed /<X>/PkgIDRef
 * at least one Operation sub-node for the Primitive to be executed
 * Deployed/<X>/Operations
 */
int check_exec_pre_condition(const char *uri, char **pname, char **parent_uri, char **command)
{
    DM_LOGI("SCOMO: check_exec_pre_condition: url <%s> ",uri);
    int rc = MO_ERROR_NONE;
    scm_node * node = NULL;

    char * token = NULL;
    char * root_node = NULL;
    char * node1 = NULL;
    char * node2 = NULL;
    char * p_uri = NULL;

    if(uri == NULL) {
        DM_LOGI("SCOMO: invalid uri");
        return MO_ERROR_NOT_EXECUTED;
    }
    if( strstr(uri,SCM_DOWNLOAD_NODE) ) {
        root_node = SCM_DOWNLOAD_NODE;
        node1 = SCM_PACKAGE_ID;
        node2 = SCM_PACKAGE_URL;
        p_uri = SCM_PARENT_URI_DOWNLOAD;
    } else if(strstr(uri,SCM_DELIVERED_NODE)) {
        root_node = SCM_DELIVERED_NODE;
        node1 = SCM_PACKAGE_ID;
        node2 = SCM_PACKAGE_DATA;
        p_uri = SCM_PARENT_URI_DELIVERED;
    } else if(strstr(uri,SCM_DEPLOYED_NODE)) {
        root_node = SCM_DEPLOYED_NODE;
        node1 = SCM_PACKAGE_IDREF;
        node2 = NULL;
        p_uri = SCM_PARENT_URI_DELIVERED;
    } else {
        DM_LOGI("SCOMO: invalid uri");
        return MO_ERROR_NOT_EXECUTED;
    }

    // if this node is exists, it's parent node /Operation is exist too.
    rc = scm_get_node(uri, &node);
    if (rc != MO_ERROR_NONE) {
        DM_LOGE("SCOMO: check_exec_pre_condition: Node not found 1");
        return rc;
    }

    if( !(node->node_command_ability_ & SCM_NODE_ABILITY_EXEC) ) {
        DM_LOGE("SCOMO: check_exec_pre_condition: Command EXEC not supported by node");
        return MO_ERROR_NOT_EXECUTED;
    }

    char * uri_cpy = strdup(uri);
    if(uri_cpy == NULL) {
        DM_LOGI("SCOMO: check_exec_pre_condition: MO_ERROR_DEVICE_FULL");
        return MO_ERROR_DEVICE_FULL;
    }

    // find package name:
    token = strtok(uri_cpy+strlen(root_node), "/");
    if(token == NULL){
        DM_LOGI("SCOMO: can't find package name uri = %s uri_cpy = %s", uri, uri_cpy);
        free(uri_cpy);
        return MO_ERROR_COMMAND_FAILED;
    } else {
        // Next token is a package name, store it
        *pname = strdup(token);
        if(*pname == NULL) {
            DM_LOGI("SCOMO: MO_ERROR_DEVICE_FULL");
            free(uri_cpy);
            return MO_ERROR_DEVICE_FULL;
         }
         // check pre-condition:

         rc = scm_get_node(uri_cpy, &node);
         if(rc != MO_ERROR_NONE) {
             DM_LOGI("SCOMO: can't get node's <%s>",uri_cpy);
             free(uri_cpy);
             return rc;
         }

         char *tmp_uri = calloc(strlen(uri_cpy)+strlen(node1)+1,sizeof(char));
         if (tmp_uri == NULL) {
            DM_LOGI("SCOMO: out of memory");
            free(uri_cpy);
            return MO_ERROR_DEVICE_FULL;
        }
        strcpy(tmp_uri,uri_cpy);
        strcat(tmp_uri,node1);
        rc = scm_get_node(tmp_uri, &node);
        if(rc != MO_ERROR_NONE) {
            DM_LOGI("SCOMO: can't get node's <%s>",tmp_uri);
            free(uri_cpy);
            free(tmp_uri);
            return rc;
        }
        free(tmp_uri);

        if(node2 != NULL) {
            tmp_uri = calloc(strlen(uri_cpy)+strlen(node2)+1,sizeof(char));
            if (tmp_uri == NULL) {
                DM_LOGI("SCOMO: out of memory");
                free(uri_cpy);
                return MO_ERROR_DEVICE_FULL;
            }
            strcpy(tmp_uri,uri_cpy);
            strcat(tmp_uri,node2);
            rc = scm_get_node(tmp_uri, &node);
            if(rc != MO_ERROR_NONE) {
                DM_LOGI("SCOMO: can't get node's <%s>",tmp_uri);
                free(uri_cpy);
                free(tmp_uri);
                return rc;
            }
            free(tmp_uri);
        }

        // find Operation
        token = strtok(NULL, "/");
        if(token == NULL) {
            DM_LOGI("SCOMO: can't find operation" );
            free(uri_cpy);
            return MO_ERROR_NOT_EXECUTED;
        }
        token = strtok(NULL, "/");
        if(token == NULL) {
            DM_LOGI("SCOMO: can't find operation" );
            free(uri_cpy);
            return MO_ERROR_NOT_EXECUTED;
        }
        *command = strdup(token);
        if(*command == NULL) {
            DM_LOGI("SCOMO: out of memory");
            free(uri_cpy);
            return MO_ERROR_DEVICE_FULL;
        }

        *parent_uri = strdup(p_uri);
        if(*parent_uri == NULL) {
            DM_LOGI("SCOMO: out of memory");
            free(uri_cpy);
            return MO_ERROR_DEVICE_FULL;
        }
    }
    free(uri_cpy);
    DM_LOGI("SCOMO: check_exec_pre_condition: exit <%d> uri %s",rc,uri);
    return rc;
}
