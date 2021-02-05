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
#include <unistd.h>

#include "fumo.h"

/** forward declaration */
int fumo_root_set_fn (const dmtree_node_t * nodeP, void * data);
int fumo_root_set_acl_fn(const char * uri, const char *acl, void * data);

/** @brief flg, !=0 if plugin already initialized */
int g_initialized = 0;

/** @brief flg, !=0 if GUI is supported by PAL */
int gui_supported = 1;

/** @brief flg, !=0 if neworking events are supported */
int network_events = 0;

/** @brief handle for network events in PAL */
void* pal_handle_network_events = NULL;

/** @brief List of functions from PAL */
const char *functions_list[] = {
        "pal_update_firmware",
        "pal_update_firmware_cancel",
        "pal_download_firmware",
        "pal_download_cancel",
        "pal_system_fwv_get",
        "pal_system_batteries_get",
        "pal_system_battery_x_level_get",
        NULL };

/** @brief List of functions from GUI PAL */
const char *functions_gui_list[] = {
        "pal_fumo_gui_create_message",
        "pal_fumo_gui_destroy_message",
        "pal_fumo_gui_set_system_update_info",
        "pal_fumo_gui_update_download_progress",
        NULL };

const fumo_nodes_offset nodes = {
        {
            {FUMO_BASE_URI, offsetof(fumo_storage_t, acl_node_root)},
            {FUMO_BASE_URI "/" FUMO_URI_PKGNAME, offsetof(fumo_storage_t, acl_pkg_name)},
            {FUMO_BASE_URI "/" FUMO_URI_PKGVERSION, offsetof(fumo_storage_t, acl_pkg_version)},
            {FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD, offsetof(fumo_storage_t, acl_download)},
            {FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_PKGURL, offsetof(fumo_storage_t, acl_download_pkg_url)},
            {FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_CANCEL, offsetof(fumo_storage_t, acl_download_cancel)},
            {FUMO_BASE_URI "/" FUMO_URI_UPDATE, offsetof(fumo_storage_t, acl_update)},
            {FUMO_BASE_URI "/" FUMO_URI_UPDATE_PKGDATA, offsetof(fumo_storage_t, acl_update_pkg_data)},
            {FUMO_BASE_URI "/" FUMO_URI_UPDATE_CANCEL, offsetof(fumo_storage_t, acl_update_cancel)},
            {FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE, offsetof(fumo_storage_t, acl_download_update)},
            {FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL, offsetof(fumo_storage_t, acl_download_update_pkg_url)},
            {FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL, offsetof(fumo_storage_t, acl_download_update_cancel)},
            {FUMO_BASE_URI "/" FUMO_URI_STATE, offsetof(fumo_storage_t, acl_state)},
            {FUMO_BASE_URI "/" FUMO_URI_CANCEL, offsetof(fumo_storage_t, acl_cancel)},
            {FUMO_BASE_URI "/" FUMO_URI_EXT, offsetof(fumo_storage_t, acl_ext)},
            {FUMO_BASE_URI "/" FUMO_URI_EXT_SEVERITY, offsetof(fumo_storage_t, acl_ext_severity)},
            {FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY, offsetof(fumo_storage_t, acl_ext_wifi_only)},
            {FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLYTIMER, offsetof(fumo_storage_t, acl_ext_wifi_only_timer)},
            {NULL, 0}
        }
    };

/**
 * @brief Load PAL functions from PAL_INSTALL_DIR "/" PAL_LIB_NAME
 * @param pal_handle handle of PAL library
 * @param list NULL-terminated list of names of functions to load
 * @return 0 if OK
 *         1 if pal_handle or list is NULL or
 *           cannot load some functions
 */
int load_pal_functions(void **pal_handle, const char **list)
{
    if(NULL == *pal_handle || NULL == *list)
        return 1;
    int i = 0;
    for (i = 0; NULL != list[i]; i++) {
        if (NULL == dlsym(*pal_handle, list[i])) {
            DM_LOGE("FUMO: Can't get %s from %s",
                    list[i],
                    PAL_INSTALL_DIR "/" PAL_LIB_NAME);
            dlclose(*pal_handle);
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Load PAL GUI functions from PAL_INSTALL_DIR "/" PAL_LIB_NAME
 * @param pal_handle handle of PAL library
 * @param list NULL-terminated list of names of functions to load
 * @return 0 if OK
 *         1 if pal_handle or list is NULL
 */
int load_pal_gui_functions(void **pal_handle, const char **list)
{
    if(NULL == *pal_handle || NULL == *list)
        return 1;
    int i = 0;
    for (i = 0; NULL != list[i]; i++) {
        if (NULL == dlsym(*pal_handle, list[i])) {
            DM_LOGE("FUMO: Can't get %s from %s",
                    list[i],
                    PAL_INSTALL_DIR "/" PAL_LIB_NAME);
            gui_supported = 0;
        }
    }
    return 0;
}

/**
 * @brief get ACL for node
 * @param[in] uri for node
 * @param[out] ACL for node
 * @return 0 if OK
 *         1 or rc if error
 */
int get_acl(char *uri, char **acl)
{
    if(NULL == uri)
        return 1;

    int rc = 0;
    int i = 0;
    *acl = NULL;

    for (i = 0; NULL != nodes.node[i].name; i++) {
        if(0 == strcmp(uri, nodes.node[i].name)) {
            if(MO_ERROR_NONE != (rc=fumo_storage_get_string_value(
                    nodes.node[i].offset,
                    PATH_MAX, acl))) {
                DM_LOGE("FUMO: get_acl: Node = %s , rc = %d", uri, rc);
                return rc;
            }
            break;
        }
    }

    return 0;
}

/**
 * @brief set ACL for node
 * @param[in] uri for node
 * @param[out] ACL for node
 * @return 0 if OK
 *         1 or rc if error
 */
int set_acl(char *uri, char *new_acl)
{
    if(NULL == uri || new_acl == NULL)
        return 1;

    int rc = 0;
    int i = 0;
    for (i = 0; NULL != nodes.node[i].name; i++) {
        if(0 == strcmp(uri, nodes.node[i].name)) {
            if(MO_ERROR_NONE != (rc=fumo_storage_set_string_value(
                    nodes.node[i].offset,
                    PATH_MAX, new_acl, strlen(new_acl)+1))) {
                DM_LOGE("FUMO: set_acl: Node = %s, ACL = %s, rc = %d", uri, new_acl, rc);
                return rc;
            }
            break;
        }
    }
    return 0;
}

/**
 * @brief Gets pointer for PAL library method
 * @param[in] handle
 * @param[in] method_name - null terminated string containing
 * pal library method name
 * @return function pointer or NULL in case of error
 */
void* get_pal_func(void **pal_handle, const char* method_name)
{
    void* fp = NULL;

    if(method_name == NULL)
        return NULL;

    if(NULL == *pal_handle && NULL == (*pal_handle=dlopen(
                        PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY))) {
        DM_LOGE("FUMO: Can't open library: %s",
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        return NULL;
    }

    if(NULL == (fp = dlsym(*pal_handle, method_name))) {
        DM_LOGE("FUMO: Can't get [%s] from %s", method_name,
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        return NULL;
    }
    return fp;
}

void fumo_omadm_restore_fn()
{
    void* pal_handle = NULL;

    DM_LOGI("FUMO: fumo_omadm_restore_fn");
    int (*fumo_omadm_restore_fc)(void) = NULL;

    if(NULL == (fumo_omadm_restore_fc = get_pal_func(&pal_handle, "android_set_property_restore"))
            || 0 != fumo_omadm_restore_fc() )
    {
        DM_LOGE("FUMO: fumo_omadm_restore_fc, can't do fumo_omadm_restore_fc");
    }

    if(pal_handle) dlclose(pal_handle);
};

/*!
 * @brief Callback to initialize the MO (MANDATORY)
 *
 * @param [out] dataP opaque pointer to MO internal data
 *
 * @returns a MO error code
 */
int fumo_root_init_fn(void **dataP)
{
    int rc = 0;
    (void)dataP;
    char path_tmp[PATH_MAX] = "";
    char *path_cur = NULL;
    size_t path_len = 0;
    pal_notify_on_network_conditions_changed_t set_newt_evens_cb = NULL;
    int (*pal_fumo_init_fc)(void) = NULL;
    void* pal_handle = NULL;

    struct stat file_stat;
    time_t rawtime = 0;
    struct tm * ptm = NULL;
    int cur_time_str_size = 0;
    char cur_time_str[40] = "";
    FILE *fd = NULL;

    DM_LOGI("FUMO: fumo_root_init_fn  g_initialized = %d",g_initialized);

    if(g_initialized) {
        DM_LOGI("FUMO: fumo_root_init_fn, already initialized");
        return MO_ERROR_NONE;
    }
	
	remove_download_flag();

    if(NULL == (pal_handle=dlopen(
                        PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY))) {
        DM_LOGE("FUMO: Can't open library: %s",
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        return MO_ERROR_COMMAND_FAILED;
    }

    if(0 != load_pal_functions(&pal_handle, functions_list) ||
            0 != load_pal_gui_functions(&pal_handle, functions_gui_list))
        return MO_ERROR_COMMAND_FAILED;

    if(NULL == dlsym(pal_handle, "pal_network_wifi_state_get")) {
        DM_LOGE("FUMO: Can't get pal_network_wifi_state_get");
    }

    if(NULL != dlsym(pal_handle,
            PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES) &&
        NULL != dlsym(pal_handle,
            PAL_NOTIFY_STOP_NETWORK_CONDITIONS_CHANGES)) {
        network_events = 1;
    } else {
        DM_LOGI("FUMO: Can't get PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES \
and PAL_NOTIFY_STOP_NETWORK_CONDITIONS_CHANGES from %s",
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
    }

    if(NULL == (pal_fumo_init_fc = dlsym(pal_handle, "pal_fumo_init")) ||
        0 != pal_fumo_init_fc()) {
        DM_LOGI("FUMO: Can't execute a pal_fumo_init");
        dlclose(pal_handle);
        return MO_ERROR_COMMAND_FAILED;
    }

    /** subscribing to network's events */
    if(0 == network_events ||  NULL == (set_newt_evens_cb = dlsym(pal_handle, PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES))
            || 0 > set_newt_evens_cb(network_conditions_changed))
        DM_LOGE("FUMO: Can't subscribe to network events");

    /** creating work directory */
    snprintf(path_tmp, sizeof(path_tmp), "%s", MO_WORK_PATH FUMO_STATE_DIR);
    path_len = strlen(path_tmp);
    if(path_tmp[path_len - 1] == '/')
        path_tmp[path_len - 1] = 0;

    for(path_cur = path_tmp + 1; *path_cur; path_cur++) {
        if(*path_cur == '/') {
            *path_cur = 0;
            mkdir(path_tmp, S_IRWXU);
            *path_cur = '/';
        }
    }
    mkdir(path_tmp, S_IRWXU);

    /** last_update_date_time creating default file */
    memset(&file_stat, 0, sizeof(file_stat));
    rc = stat(MO_WORK_PATH FUMO_STATE_DIR PAL_FILENAME_LAST_UPD_DATETIME,
        &file_stat);
	DM_LOGE(" start restore rc = %d   st_size = %d\n", rc, file_stat.st_size);
    if(-1 == rc || 0 == file_stat.st_size) {
        if(NULL == (fd=fopen(
            MO_WORK_PATH FUMO_STATE_DIR PAL_FILENAME_LAST_UPD_DATETIME, "w"))) {
                DM_LOGE("FUMO: Can't create default file: %s%s%s",
                    MO_WORK_PATH, FUMO_STATE_DIR,
                    PAL_FILENAME_LAST_UPD_DATETIME);
            return MO_ERROR_COMMAND_FAILED;
        }
        time ( &rawtime );
        ptm = gmtime ( &rawtime );
        cur_time_str_size = strftime(cur_time_str, sizeof(cur_time_str),
            PAL_DATEIME_FILE_FORMAT, ptm);
        fwrite(cur_time_str, 1, cur_time_str_size, fd);
        fclose(fd);
//barret
		//fumo_omadm_restore_fn();  //remove by chapin, it don't do factory reset
    }

    /** creating default firmware version file */
    memset(&file_stat, 0, sizeof(file_stat));
    rc = stat(MO_WORK_PATH FUMO_STATE_DIR PAL_FILENAME_LAST_UPD_FIRMWARE,
        &file_stat);
    if(-1 == rc || 0 == file_stat.st_size) {
        if(NULL == (fd=fopen(
            MO_WORK_PATH FUMO_STATE_DIR PAL_FILENAME_LAST_UPD_FIRMWARE, "w"))) {
                DM_LOGE("FUMO: Can't create default file: %s%s%s",
                    MO_WORK_PATH, FUMO_STATE_DIR,
                    PAL_FILENAME_LAST_UPD_FIRMWARE);
            return MO_ERROR_COMMAND_FAILED;
        }
        fwrite(PAL_DEFAULT_FIRMWARE_VERSION, 1,
            strlen(PAL_DEFAULT_FIRMWARE_VERSION), fd);
        fclose(fd);
    }

    /** creating default firmware package name file */
    memset(&file_stat, 0, sizeof(file_stat));
    rc = stat(MO_WORK_PATH FUMO_STATE_DIR PAL_FILENAME_FIRMWARE_PACKET_NAME,
        &file_stat);
    if(-1 == rc || 0 == file_stat.st_size) {
        if(NULL == (fd=fopen(
            MO_WORK_PATH FUMO_STATE_DIR PAL_FILENAME_FIRMWARE_PACKET_NAME,
            "w"))) {
                DM_LOGE("FUMO: Can't create default file: %s%s%s",
                    MO_WORK_PATH, FUMO_STATE_DIR,
                    PAL_FILENAME_FIRMWARE_PACKET_NAME);
            return MO_ERROR_COMMAND_FAILED;
        }
        fwrite(PAL_DEFAULT_FIRMWARE_PACKET_NAME, 1,
            strlen(PAL_DEFAULT_FIRMWARE_PACKET_NAME), fd);
        fclose(fd);
    }

    /** restoring last saved state*/
    int rcc = fumo_continue_job(NULL);
    if( rcc != MO_ERROR_NONE && rcc != FUMO_RC_PRV_UPDATE_DEFERRED) {
        DM_LOGE("FUMO: can't restore previous state");
        return MO_ERROR_COMMAND_FAILED;
    }

	if(0 == access("/cache/update_flag_complete_ui", F_OK)) {
       	show_update_complete_dialog();
		DM_LOGI("check_update update_flag_complete_ui\n");
   	}

    g_initialized = 1;
    if(pal_handle) dlclose(pal_handle);

    return MO_ERROR_NONE;
};

int network_conditions_changed(network_conditions_t nc)
{
    DM_LOGI("FUMO: network_conditions_changed, net_feature=%d, enabled=%d",
            nc.net_feature, nc.enabled);
	int net_stat = 0;

    if (nc.enabled) {
        DM_LOGI("FUMO: condition changed net is %d" , nc.net_feature);
        /** Wi-Fi hanlder */
        if (nc.net_feature == NETWORK_WIFI_CONNECTED) {
            pthread_mutex_lock(&wifi_wait_mutex);
            pthread_cond_signal(&wifi_wait_conditional);
            pthread_mutex_unlock(&wifi_wait_mutex);
            DM_LOGI("FUMO: Wi-Fi connected");
        }
    } else {
        DM_LOGI("FUMO: network disconnected send signal for timeout condition variable, may be wifi disconnect, try open admin, then wait");
        /*pthread_mutex_lock(&retry_timeout_mutex);
        pthread_cond_signal(&retry_timeout);
        pthread_mutex_unlock(&retry_timeout_mutex);*/
    }

    return MO_ERROR_NONE;
}

/*!
 * @brief Callback to free the MO
 *
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 */
void fumo_root_close_fn(void *data)
{
    (void) data;
    void* pal_handle = NULL;

    DM_LOGI("FUMO: fumo_root_close_fn");
    int (*pal_fumo_close_fc)(void) = NULL;

    if(NULL == (pal_fumo_close_fc = get_pal_func(&pal_handle, "pal_fumo_close"))
            || 0 != pal_fumo_close_fc() )
    {
        DM_LOGE("FUMO: fumo_root_close_fn, can't do pal_fumo_close");
    }

    if(pal_handle) dlclose(pal_handle);
};

/*!
 * @brief Callback to get the type of a node
 *
 * @param [in] uri URL of the node
 * @param [out] type type of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_is_node_fn(const char *uri, omadmtree_node_kind_t *type,
                                void *data)
{
    (void)data;
    int rc = MO_ERROR_NOT_EXECUTED;
    fumo_node *node = NULL;

    if(!type) {
        DM_LOGE("FUMO: fumo_root_is_node_fn: Invalid parameter type=NULL");
        return MO_ERROR_COMMAND_FAILED;
    }

    *type = OMADM_NODE_NOT_EXIST;

    if(!uri || *uri == '\0') {
        DM_LOGE("FUMO: fumo_root_is_node_fn: Empty uri");
        return MO_ERROR_NONE;
    }

    rc = find_node(uri, &node);
    if(rc == MO_ERROR_NONE)
        *type = node->node_kind_;

    DM_LOGI("FUMO: fumo_root_is_node_fn: Node type = %d", *type);

    return rc;
};

/*!
 * @brief Callback to find the URLs associated to an URN
 *
 * @param [in] urn URN to find
 * @param [out] urlsP null-terminated array of urls, freed by the caller
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_find_urn_fn (const char * urn, char *** urlsP, void * data)
{
    (void)data;
    int i = 0, j = 0, count = 0, deviceFull = 0;
    char **urnArray = NULL;

    DM_LOGI("FUMO: fumo_root_find_urn_fn {");
    if(!urn || *urn == '\0' || !urlsP ) {
        DM_LOGE("FUMO: fumo_root_find_urn_fn: invalid parameters");
        return MO_ERROR_COMMAND_FAILED;
    }

    *urlsP = NULL;

    DM_LOGI("FUMO: fumo_root_find_urn_fn urn=%s", urn);
    for(i=0; i < NODES_COUNT; ++i) {
        if(g_fumo_static_nodes.nodes_[i].uri_ == NULL)
            break;
        if(g_fumo_static_nodes.nodes_[i].urn_ == NULL)
            continue;
        if( strcmp(urn, g_fumo_static_nodes.nodes_[i].urn_) != 0)
            continue;

        urnArray = (char**)realloc( *urlsP, sizeof(char*)*(count+1) );
        if(!urnArray) {
            deviceFull = 1;
            break;
        }
        urnArray[count] = strdup(g_fumo_static_nodes.nodes_[i].uri_);
        if(!urnArray[count]) {
            deviceFull = 1;
            break;
        }
        count++;
        *urlsP = urnArray;
    }
    urnArray = (char**)realloc( *urlsP, sizeof(char*)*(count+1) );
    urnArray[count] = NULL;

    if(deviceFull) {
       if(*urlsP) {
           for(j=0; j < count; ++j) {
               free( (*urlsP)[j] );
           }
           free(*urlsP);
       }
       DM_LOGE("FUMO: fumo_root_find_urn_fn: Out of memory");
       return MO_ERROR_DEVICE_FULL;
    }

    if(!count) {
        DM_LOGE("FUMO: fumo_root_find_urn_fn: urn not found");
        return MO_ERROR_NOT_FOUND;
    }

    return MO_ERROR_NONE;
};

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
int fumo_root_get_fn (dmtree_node_t * nodeP, void * data)
{
    (void)data;
    fumo_node *node = NULL;
    int rc = MO_ERROR_NOT_EXECUTED;
    unsigned int i = 0;

    DM_LOGI("FUMO: fumo_root_get_fn {");
    if(!nodeP || !nodeP->uri ) {
        DM_LOGE("FUMO: fumo_root_get_fn: Invalid parameters");
        return MO_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("FUMO: fumo_root_get_fn uri=%s", nodeP->uri);
    rc = find_node(nodeP->uri, &node);
    if(rc != MO_ERROR_NONE) {
        DM_LOGE("FUMO: fumo_root_get_fn: Node not found");
        return rc;
    }

    if( ! (node->node_command_ability_ & FUMO_NODE_ABILITY_GET) ) {
        DM_LOGE("FUMO: fumo_root_get_fn: Command GET not supported by node");
        return MO_ERROR_FORBIDDEN;
    }

    if(get_string_node_format(node->format_, &nodeP->format) != 0) {
        DM_LOGE("FUMO: fumo_root_get_fn: Out of memory");
        return MO_ERROR_DEVICE_FULL;
    }

    if(get_string_node_type(node->format_, &nodeP->type) != 0) {
        free(nodeP->format);
        nodeP->format = NULL;
        DM_LOGE("FUMO: fumo_root_get_fn: Out of memory");
        return MO_ERROR_DEVICE_FULL;
    }

    rc = get_node_value(node, &nodeP->data_size, &nodeP->data_buffer);
    if(rc != MO_ERROR_NONE) {
        free(nodeP->format);
        nodeP->format = NULL;
        if(nodeP->type) {
            free(nodeP->type);
            nodeP->type = NULL;
        }
        nodeP->data_size = 0;
        nodeP->data_buffer = NULL;
        DM_LOGE("FUMO: fumo_root_get_fn: Could not get node value");
    } else {
        DM_LOGI("FUMO: fumo_root_get_fn: data-size = %d", nodeP->data_size);
        for(i=0; i<nodeP->data_size; ++i) {
            DM_LOGI("[%02X] ", (0xFF&(nodeP->data_buffer[i])));
        }
    }

    return rc;
};

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
int fumo_root_set_fn (const dmtree_node_t * nodeP, void * data)
{
    (void)data;
    fumo_node *node = NULL;
    int rc = MO_ERROR_NOT_EXECUTED;

    DM_LOGI("FUMO: fumo_root_set_fn {");
    if(!nodeP || !nodeP->uri ) {
        DM_LOGE("FUMO: fumo_root_set_fn: Invalid parameters");
        return MO_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("FUMO: fumo_root_set_fn uri=%s", nodeP->uri);
    rc = find_node(nodeP->uri, &node);
    if (rc == MO_ERROR_NOT_FOUND) {
        /* TODO: calling create node function */
        DM_LOGE("FUMO: fumo_root_set_fn: Node not found");
        rc = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
    } else if(rc == MO_ERROR_NONE) {
        if( ! (node->node_command_ability_ & FUMO_NODE_ABILITY_REPLACE) ) {
            DM_LOGE("FUMO: fumo_root_set_fn: Command SET not supported by node");
            return MO_ERROR_FORBIDDEN;
        }

        rc = set_node_value(node, nodeP->data_size, nodeP->data_buffer);
        DM_LOGE("FUMO: fumo_root_set_fn: data=[%s], size=%d",
            nodeP->data_buffer, nodeP->data_size);
        DM_LOGE("FUMO: fumo_root_set_fn: set_node_value return code %d", rc);
    }

    return rc;
};

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
int fumo_root_get_acl_fn (const char * uri, char ** aclP, void * data)
{
    (void)data;
    fumo_node *node = NULL;
    int rc = MO_ERROR_NOT_EXECUTED;

    DM_LOGI("FUMO: fumo_root_get_acl_fn {");
    if(!uri || *uri == '\0') {
        DM_LOGE("FUMO: fumo_root_get_acl_fn: Invalid uri");
        return MO_ERROR_NONE;
    }

    if(!aclP) {
        DM_LOGE("FUMO: fumo_root_get_acl_fn: aclP is NULL");
        return MO_ERROR_COMMAND_FAILED;
    }

    *aclP = NULL;

    DM_LOGI("FUMO: fumo_root_get_acl_fn uri=%s", uri);
    rc = find_node(uri, &node);
    if(rc != MO_ERROR_NONE) {
        DM_LOGE("FUMO: fumo_root_get_acl_fn: Node not found");
        return rc;
    }

    if(0 != (rc = get_acl(node->uri_, aclP))) {
        return rc;
    }

    DM_LOGE("FUMO: fumo_root_get_acl_fn: ACL = %s", *aclP);

    return MO_ERROR_NONE;
};

/*!
 * @brief Callback to set the ACL of a node
 *
 * @param [in] uri URL of the node
 * @param [in] acl ACL of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_set_acl_fn(const char * uri, const char *acl, void * data)
{
    (void)data;
    fumo_node *node = NULL;
    int rc = MO_ERROR_NOT_EXECUTED;
    char *new_acl = NULL;

    DM_LOGI("FUMO: fumo_root_set_acl_fn {");
    if(!uri || *uri == '\0') {
        DM_LOGE("FUMO: fumo_root_set_acl_fn: Invalid uri");
        return MO_ERROR_NONE;
    }

    if(!acl) {
        DM_LOGE("FUMO: fumo_root_set_acl_fn: acl is NULL");
        return MO_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("FUMO: fumo_root_set_acl_fn uri=%s, acl=%s", uri, acl);
    rc = find_node(uri, &node);
    if(rc != MO_ERROR_NONE) {
        DM_LOGE("FUMO: fumo_root_set_acl_fn: Node not found");
        return rc;
    }

    new_acl = strdup(acl);
    if(NULL == new_acl) return MO_ERROR_DEVICE_FULL;

    if(0 != (rc = set_acl(node->uri_, new_acl))) {
        return rc;
    }

    DM_LOGE("FUMO: fumo_root_set_acl_fn: acl = %s", new_acl);

    return MO_ERROR_NONE;
};

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
int fumo_root_exec_fn (const char * uri, const char * cmdData,
                             const char * correlator, void * data)
{
    (void)data;
    fumo_node *node = NULL;
    int rc = MO_ERROR_NOT_EXECUTED;

    DM_LOGI("FUMO: fumo_root_exec_fn {");
    if(!uri || *uri == '\0') {
        DM_LOGE("FUMO: fumo_root_exec_fn: Invalid uri");
        return MO_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("FUMO: fumo_root_exec_fn uri=%s", uri);
    rc = find_node(uri, &node);
    if (rc != MO_ERROR_NONE) {
        DM_LOGE("FUMO: fumo_root_exec_fn: Node not found");
        return rc;
    }

    if( !(node->node_command_ability_ & FUMO_NODE_ABILITY_EXEC) ) {
        DM_LOGE("FUMO: fumo_root_exec_fn: Command EXEC not supported by node");
        return MO_ERROR_NOT_EXECUTED;
    }

    rc = node_command_exec(node, cmdData, correlator);
    if(rc != FUMO_RC_ACCEPTED_FOR_PROCESSING)
        DM_LOGE("FUMO: fumo_root_exec_fn: failed with error code %d", rc);

    DM_LOGI("FUMO: fumo_root_exec_fn }");
    return rc;
};

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
        (omadm_mo_interface_t*) calloc(1, sizeof(*interface) );
    if( !interface ) return NULL;

    memset(interface, 0, sizeof(omadm_mo_interface_t));

    interface->base_uri = strdup(FUMO_BASE_URI);
    interface->initFunc = fumo_root_init_fn;
    interface->closeFunc = fumo_root_close_fn;
    interface->isNodeFunc = fumo_root_is_node_fn;
    interface->findURNFunc  = fumo_root_find_urn_fn;
    interface->getFunc = fumo_root_get_fn;
    interface->setFunc = fumo_root_set_fn;
    interface->getACLFunc = fumo_root_get_acl_fn;
    interface->setACLFunc = fumo_root_set_acl_fn;
    interface->execFunc = fumo_root_exec_fn;

    DM_LOGI("FUMO: omadm_get_mo_interface: interface = %p", interface);
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

    DM_LOGI("FUMO: omadm_mo_register_event_handler {");
    if( g_dmclient_event_handler || event_handler == NULL ) {
        DM_LOGE("FUMO: omadm_mo_register_event_handler } failed");
        return 1;
    } else {
        g_dmclient_event_handler = event_handler;
        DM_LOGI("FUMO: omadm_mo_register_event_handler }");
        return 0;
    }
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

    DM_LOGI("FUMO: omadm_mo_unregister_event_handler {");
    if(g_dmclient_event_handler != event_handler || event_handler == NULL) {
        DM_LOGE("FUMO: omadm_mo_unregister_event_handler } failed");
        return 1;
    } else {
        g_dmclient_event_handler = NULL;
        DM_LOGI("FUMO: omadm_mo_unregister_event_handler }");
        return 0;
    }
}

/** @todo removes all temporary files and sets state to default */
void fumo_cleanup(void)
{
    fumo_storage_set_string_value(offsetof(fumo_storage_t, active_node),
        NAME_MAX, NULL, 0);
    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
}

