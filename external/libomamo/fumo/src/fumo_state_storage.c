/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "pal.h"
#include "fumo.h"

/**
    @param g_fumo_storage memory mapped structure pointer to
        MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE
*/
static fumo_storage_t* g_fumo_storage_mm = (fumo_storage_t*)MAP_FAILED;

/**
    @param g_fumo_storage_file memory mapped file for g_fumo_storage
*/
static int g_fumo_storage_file = -1;

/** @brief get version from package name */
int get_version_from_name_pkg(char* fwname);

/** @brief fumo_storage_open forward declaration */
int fumo_storage_open(void);
/** @brief fumo_storage_close forward declaration */
int fumo_storage_close(void);

pthread_mutex_t storage_wait_mutex = PTHREAD_MUTEX_INITIALIZER;

int fumo_storage_get_pkg_name(char **pkg_name)
{
    DM_LOGI("FUMO: fumo_storage_get_pkg_name {");
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;

    if(pkg_name == NULL) {
        DM_LOGE("FUMO: fumo_storage_get_pkg_name ERROR_INCOMPLETE_COMMAND");
        return MO_ERROR_INCOMPLETE_COMMAND;
    }

    *pkg_name = NULL;
    pthread_mutex_lock(&storage_wait_mutex);

    do {
        if(MO_ERROR_NONE != (rc=fumo_storage_open())) {
            DM_LOGE("FUMO: fumo_storage_get_pkg_name fumo_storage_open rc=%d", rc);
            break;
        }

        if('\0' == g_fumo_storage_mm->pkg_name[0] || !strlen(g_fumo_storage_mm->pkg_name)) {
            DM_LOGE("FUMO: fumo_storage_get_pkg_name pkg_name is absent");
            rc = MO_ERROR_NOT_FOUND;
            break;
        }

        if(NULL == (*pkg_name=strdup(g_fumo_storage_mm->pkg_name))) {
            DM_LOGE("FUMO: fumo_storage_get_pkg_name ERROR_DEVICE_FULL");
            rc = MO_ERROR_DEVICE_FULL;
            break;
        }
        rc = MO_ERROR_NONE;
    } while(0);

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_get_pkg_name fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO: fumo_storage_get_pkg_name }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return rc;
};

int fumo_storage_set_pkg_name(char *pkg_name)
{
    DM_LOGI("FUMO: fumo_storage_set_pkg_name {");
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;

    if(pkg_name == NULL) {
        DM_LOGE("FUMO: fumo_storage_set_pkg_name ERROR_INCOMPLETE_COMMAND");
        return MO_ERROR_INCOMPLETE_COMMAND;
    }
    pthread_mutex_lock(&storage_wait_mutex);
    if(MO_ERROR_NONE != (rc=fumo_storage_open())) {
        DM_LOGE("FUMO: fumo_storage_set_pkg_name fumo_storage_open rc=%d", rc);
    } else {
        strncpy(g_fumo_storage_mm->pkg_name, pkg_name, sizeof(g_fumo_storage_mm->pkg_name));
    }

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_set_pkg_name fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO: fumo_storage_set_pkg_name }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return rc;
};


int fumo_storage_get_pkg_version(char **pkg_version)
{
    DM_LOGI("FUMO: fumo_storage_get_pkg_version {");
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;

    if(pkg_version == NULL) {
        DM_LOGE("FUMO: fumo_storage_get_pkg_version INCOMPLETE_COMMAND");
        return MO_ERROR_INCOMPLETE_COMMAND;
    }

    *pkg_version = NULL;
    pthread_mutex_lock(&storage_wait_mutex);
    do {
        if(MO_ERROR_NONE != (rc=fumo_storage_open())) {
            DM_LOGE("FUMO: fumo_storage_get_pkg_version fumo_storage_open rc=%d", rc);
            break;
        }

        if('\0' == g_fumo_storage_mm->pkg_version[0] || !strlen(g_fumo_storage_mm->pkg_version)) {
            DM_LOGE("FUMO: fumo_storage_get_pkg_version pkg_version is absent");
            rc = MO_ERROR_NOT_FOUND;
            break;
        }

        if(NULL == (*pkg_version=strdup(g_fumo_storage_mm->pkg_version))) {
            DM_LOGE("FUMO: fumo_storage_get_pkg_version ERROR_DEVICE_FULL");
            rc = MO_ERROR_DEVICE_FULL;
            break;
        }
        rc = MO_ERROR_NONE;
    } while(0);

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_get_pkg_version fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO: fumo_storage_get_pkg_version }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return rc;
};

int fumo_storage_set_pkg_version(char *pkg_version)
{
    DM_LOGI("FUMO: fumo_storage_set_pkg_version {");
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;

    if(pkg_version == NULL) {
        DM_LOGE("FUMO: fumo_storage_set_pkg_version ERROR_INCOMPLETE_COMMAND");
        return MO_ERROR_INCOMPLETE_COMMAND;
    }
    pthread_mutex_lock(&storage_wait_mutex);
    if( MO_ERROR_NONE != (rc=fumo_storage_open())) {
        DM_LOGE("FUMO: fumo_storage_set_pkg_version fumo_storage_open rc=%d", rc);
    } else {
        strncpy(g_fumo_storage_mm->pkg_version, pkg_version, sizeof(g_fumo_storage_mm->pkg_version));
    }

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_set_pkg_version fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO: fumo_storage_set_pkg_version }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return rc;
};

int fumo_storage_get_update_descriptor(
                                pal_update_descriptor_t *update_descriptor)
{
    DM_LOGI("FUMO: fumo_storage_get_update_descriptor {");
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;

    if(NULL == update_descriptor) {
        DM_LOGE("FUMO: fumo_storage_get_update_descriptor ERROR_INCOMPLETE_COMMAND");
        return MO_ERROR_INCOMPLETE_COMMAND;
    }

    memset(update_descriptor, 0, sizeof(pal_update_descriptor_t));
    pthread_mutex_lock(&storage_wait_mutex);
    do {
        if(MO_ERROR_NONE != (rc=fumo_storage_open())) {
            DM_LOGE("FUMO: fumo_storage_get_update_descriptor fumo_storage_open rc=%d", rc);
            break;
        }

        if('\0' == g_fumo_storage_mm->pkg_name[0]) {
                DM_LOGE("FUMO: fumo_storage_get_update_descriptor pkg name NOT_FOUND");
                pthread_mutex_unlock(&storage_wait_mutex);
                return MO_ERROR_NOT_FOUND;
        }

        if(NULL == (update_descriptor->name = (char*)malloc(
                strlen(MO_WORK_PATH  FUMO_STATE_DIR)+strlen(g_fumo_storage_mm->pkg_name)+1))) {
                DM_LOGE("FUMO: fumo_storage_get_update_descriptor ERROR_DEVICE_FULL");
                pthread_mutex_unlock(&storage_wait_mutex);
                return MO_ERROR_DEVICE_FULL;
        }

        snprintf(update_descriptor->name,
                strlen(MO_WORK_PATH FUMO_STATE_DIR)+strlen(g_fumo_storage_mm->pkg_name)+1,
                "%s%s", MO_WORK_PATH FUMO_STATE_DIR, g_fumo_storage_mm->pkg_name);

        update_descriptor->size = g_fumo_storage_mm->size;
        update_descriptor->context = g_fumo_storage_mm->context;

        if('\0' != g_fumo_storage_mm->type[0] ) {
            if(NULL == (update_descriptor->type=strdup(g_fumo_storage_mm->type))) {
                rc = MO_ERROR_DEVICE_FULL;
                break;
            }
        }

        if('\0' != g_fumo_storage_mm->vendor[0] ) {
            if(NULL == (update_descriptor->vendor=strdup(g_fumo_storage_mm->vendor))) {
                DM_LOGE("FUMO: fumo_storage_get_update_descriptor ERROR_DEVICE_FULL");
                rc = MO_ERROR_DEVICE_FULL;
                break;
            }
        }

        if('\0' != g_fumo_storage_mm->install_param[0] ) {
            if(NULL == (update_descriptor->install_param=strdup(g_fumo_storage_mm->install_param))) {
                DM_LOGE("FUMO: fumo_storage_get_update_descriptor ERROR_DEVICE_FULL");
                rc =  MO_ERROR_DEVICE_FULL;
                break;
            }
        }
        rc = MO_ERROR_NONE;
    } while(0);

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_get_update_descriptor fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO: fumo_storage_get_update_descriptor }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return rc;
};

int fumo_storage_set_download_descriptor(
                                pal_download_descriptor_t *download_descriptor)
{
    DM_LOGI("FUMO: fumo_storage_set_download_descriptor {");

    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;

    if(download_descriptor == NULL) {
        DM_LOGE("FUMO: fumo_storage_set_download_descriptor ERROR_INCOMPLETE_COMMAND");
        return MO_ERROR_INCOMPLETE_COMMAND;
    }
    pthread_mutex_lock(&storage_wait_mutex);
    if( MO_ERROR_NONE != (rc=fumo_storage_open())) {
        DM_LOGE("FUMO: fumo_storage_set_download_descriptor fumo_storage_open rc=%d", rc);
    } else {
        memset(g_fumo_storage_mm->pkg_name, 0, sizeof(g_fumo_storage_mm->pkg_name));
        if( download_descriptor->packageName &&
                                    '\0' != download_descriptor->packageName[0] )
            strncpy(g_fumo_storage_mm->pkg_name, download_descriptor->packageName,
                sizeof(g_fumo_storage_mm->pkg_name));

        memset(g_fumo_storage_mm->pkg_version, 0, sizeof(g_fumo_storage_mm->pkg_version));
        if( download_descriptor->ddVersion &&
                                    '\0' != download_descriptor->ddVersion[0] )
            strncpy(g_fumo_storage_mm->pkg_version, download_descriptor->ddVersion,
                sizeof(g_fumo_storage_mm->pkg_version));
        else if(download_descriptor->packageName &&
                '\0' != download_descriptor->packageName[0] ) {
            // get default version from package name
            get_version_from_name_pkg(download_descriptor->packageName);
        }

        memset(g_fumo_storage_mm->object_uri, 0, sizeof(g_fumo_storage_mm->object_uri));
        if( download_descriptor->objectUri &&
                                    '\0' != download_descriptor->objectUri[0] )
            strncpy(g_fumo_storage_mm->object_uri, download_descriptor->objectUri,
                sizeof(g_fumo_storage_mm->object_uri));

        g_fumo_storage_mm->size = download_descriptor->size;
        g_fumo_storage_mm->required_install_parametr = download_descriptor->requiredInstall;

        memset(g_fumo_storage_mm->type, 0, sizeof(g_fumo_storage_mm->type));
        if( download_descriptor->type &&
                                    '\0' != download_descriptor->type[0] )
            strncpy(g_fumo_storage_mm->type, download_descriptor->type,
                sizeof(g_fumo_storage_mm->type));

        memset(g_fumo_storage_mm->vendor, 0, sizeof(g_fumo_storage_mm->vendor));
        if( download_descriptor->vendor &&
                                    '\0' != download_descriptor->vendor[0] )
            strncpy(g_fumo_storage_mm->vendor, download_descriptor->vendor,
                sizeof(g_fumo_storage_mm->vendor));

        memset(g_fumo_storage_mm->description, 0, sizeof(g_fumo_storage_mm->description));
        if( download_descriptor->description &&
                                    '\0' != download_descriptor->description[0] )
            strncpy(g_fumo_storage_mm->description, download_descriptor->description,
                sizeof(g_fumo_storage_mm->description));

        memset(g_fumo_storage_mm->install_param, 0, sizeof(g_fumo_storage_mm->install_param));
        if( download_descriptor->installParam &&
                                    '\0' != download_descriptor->installParam[0] )
            strncpy(g_fumo_storage_mm->install_param, download_descriptor->installParam,
                sizeof(g_fumo_storage_mm->install_param));

        memset(g_fumo_storage_mm->pre_download_message, 0, sizeof(g_fumo_storage_mm->pre_download_message));
        if( download_descriptor->preDownloadMessage &&
                '\0' != download_descriptor->preDownloadMessage[0] )
            strncpy(g_fumo_storage_mm->pre_download_message, download_descriptor->preDownloadMessage,
                            sizeof(g_fumo_storage_mm->pre_download_message));

        memset(g_fumo_storage_mm->post_download_message, 0, sizeof(g_fumo_storage_mm->post_download_message));
        if( download_descriptor->postDownloadMessage &&
                 '\0' != download_descriptor->postDownloadMessage[0] )
            strncpy(g_fumo_storage_mm->post_download_message, download_descriptor->postDownloadMessage,
                            sizeof(g_fumo_storage_mm->post_download_message));

        memset(g_fumo_storage_mm->pre_download_url, 0, sizeof(g_fumo_storage_mm->pre_download_url));
        if( download_descriptor->preDownloadURL &&
                  '\0' != download_descriptor->preDownloadURL[0] )
            strncpy(g_fumo_storage_mm->pre_download_url, download_descriptor->preDownloadURL,
                            sizeof(g_fumo_storage_mm->pre_download_url));

        memset(g_fumo_storage_mm->post_download_url, 0, sizeof(g_fumo_storage_mm->post_download_url));
        if( download_descriptor->postDownloadURL &&
                   '\0' != download_descriptor->postDownloadURL[0] )
            strncpy(g_fumo_storage_mm->post_download_url, download_descriptor->postDownloadURL,
                            sizeof(g_fumo_storage_mm->post_download_url));

        memset(g_fumo_storage_mm->post_update_url, 0, sizeof(g_fumo_storage_mm->post_update_url));
        if( download_descriptor->postUpdateURL &&
                    '\0' != download_descriptor->postUpdateURL[0] )
            strncpy(g_fumo_storage_mm->post_update_url, download_descriptor->postUpdateURL,
                            sizeof(g_fumo_storage_mm->post_update_url));

        memset(g_fumo_storage_mm->post_update_message, 0, sizeof(g_fumo_storage_mm->post_update_message));
        if( download_descriptor->postUpdateMessage &&
                    '\0' != download_descriptor->postUpdateMessage[0] )
            strncpy(g_fumo_storage_mm->post_update_message, download_descriptor->postUpdateMessage,
                            sizeof(g_fumo_storage_mm->post_update_message));
    }

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_set_download_descriptor fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO: fumo_storage_set_download_descriptor }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return rc;
};

int fumo_storage_get_state(int *state)
{
    DM_LOGI("FUMO: fumo_storage_get_state {");
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;

    if(state == NULL) {
        DM_LOGE("FUMO: fumo_storage_get_state ERROR_INCOMPLETE_COMMAND");
        return MO_ERROR_INCOMPLETE_COMMAND;
    }
    pthread_mutex_lock(&storage_wait_mutex);
    *state = FUMO_DEVICE_STATE_IDLE;
    do {
        if(MO_ERROR_NONE != (rc=fumo_storage_open())) {
            DM_LOGE("FUMO: fumo_storage_get_state fumo_storage_open rc=%d", rc);
            break;
        }
        DM_LOGI("FUMO: fumo_storage_get_state g_fumo_storage_mm = %p",g_fumo_storage_mm);

         *state = g_fumo_storage_mm->state;
         rc = MO_ERROR_NONE;
    } while(0);

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_get_state fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO: fumo_storage_get_state } state = %d",*state);
    pthread_mutex_unlock(&storage_wait_mutex);
    return rc;
}

int fumo_storage_set_state(int state)
{
    DM_LOGI("FUMO: fumo_storage_set_state { state = %d",state);
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;
    pthread_mutex_lock(&storage_wait_mutex);
    if( MO_ERROR_NONE != (rc=fumo_storage_open())) {
        DM_LOGE("FUMO: fumo_storage_set_state fumo_storage_open rc=%d", rc);
    } else {
        g_fumo_storage_mm->state = state;
    }

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_set_state fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO: fumo_storage_set_state }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return rc;
}

int fumo_storage_init()
{
    DM_LOGI("FUMO: fumo_storage_init: {");
    char *acl_get = "Get=*";
    char *acl_exec = "Exec=*";
    char *acl_get_exec = "Get=*&Exec=*";
    char *acl_get_replace = "Get=*&Replace=*";

    g_fumo_storage_mm->state = FUMO_DEVICE_STATE_IDLE;
    g_fumo_storage_mm->wifi_only = 0;
    g_fumo_storage_mm->wifi_only_timer = 23;
    g_fumo_storage_mm->wifi_only_check_timer = 0;
    g_fumo_storage_mm->restart_download = 0;
    g_fumo_storage_mm->retry_count = 0;
    g_fumo_storage_mm->required_space_for_update = 0;
    g_fumo_storage_mm->required_space_for_delete = 0;

    g_fumo_storage_mm->automatic_update_enable = 0;
    g_fumo_storage_mm->defer_time = 0;

    memset(g_fumo_storage_mm->severity, 0, sizeof(g_fumo_storage_mm->severity));
    memset(g_fumo_storage_mm->acl_node_root, 0, sizeof(g_fumo_storage_mm->acl_node_root));
    memset(g_fumo_storage_mm->acl_pkg_name, 0, sizeof(g_fumo_storage_mm->acl_pkg_name));
    memset(g_fumo_storage_mm->acl_pkg_version, 0, sizeof(g_fumo_storage_mm->acl_pkg_version));
    memset(g_fumo_storage_mm->acl_download, 0, sizeof(g_fumo_storage_mm->acl_download));
    memset(g_fumo_storage_mm->acl_download_pkg_url, 0, sizeof(g_fumo_storage_mm->acl_download_pkg_url));
    memset(g_fumo_storage_mm->acl_update, 0, sizeof(g_fumo_storage_mm->acl_update));
    memset(g_fumo_storage_mm->acl_update_pkg_data, 0, sizeof(g_fumo_storage_mm->acl_update_pkg_data));
    memset(g_fumo_storage_mm->acl_download_update, 0, sizeof(g_fumo_storage_mm->acl_download_update));
    memset(g_fumo_storage_mm->acl_download_update_pkg_url, 0, sizeof(g_fumo_storage_mm->acl_download_update_pkg_url));
    memset(g_fumo_storage_mm->acl_state, 0, sizeof(g_fumo_storage_mm->acl_state));
    memset(g_fumo_storage_mm->acl_ext, 0, sizeof(g_fumo_storage_mm->acl_ext));
    memset(g_fumo_storage_mm->acl_ext_severity, 0, sizeof(g_fumo_storage_mm->acl_ext_severity));
    memset(g_fumo_storage_mm->acl_ext_wifi_only, 0, sizeof(g_fumo_storage_mm->acl_ext_wifi_only));
    memset(g_fumo_storage_mm->acl_ext_wifi_only_timer, 0, sizeof(g_fumo_storage_mm->acl_ext_wifi_only_timer));

    memset(g_fumo_storage_mm->acl_cancel, 0, sizeof(g_fumo_storage_mm->acl_cancel));
    memset(g_fumo_storage_mm->acl_update_cancel, 0, sizeof(g_fumo_storage_mm->acl_update_cancel));
    memset(g_fumo_storage_mm->acl_download_cancel, 0, sizeof(g_fumo_storage_mm->acl_download_cancel));
    memset(g_fumo_storage_mm->acl_download_update_cancel, 0, sizeof(g_fumo_storage_mm->acl_download_update_cancel));

    strncpy(g_fumo_storage_mm->acl_node_root, acl_get, sizeof(g_fumo_storage_mm->acl_node_root));
    strncpy(g_fumo_storage_mm->acl_pkg_name, acl_get, sizeof(g_fumo_storage_mm->acl_pkg_name));
    strncpy(g_fumo_storage_mm->acl_pkg_version, acl_get, sizeof(g_fumo_storage_mm->acl_pkg_version));
    strncpy(g_fumo_storage_mm->acl_download, acl_get_exec, sizeof(g_fumo_storage_mm->acl_download));
    strncpy(g_fumo_storage_mm->acl_download_pkg_url, acl_get_replace, sizeof(g_fumo_storage_mm->acl_download_pkg_url));
    strncpy(g_fumo_storage_mm->acl_update, acl_get_exec, sizeof(g_fumo_storage_mm->acl_update));
    strncpy(g_fumo_storage_mm->acl_update_pkg_data, acl_get, sizeof(g_fumo_storage_mm->acl_update_pkg_data));
    strncpy(g_fumo_storage_mm->acl_download_update, acl_get_exec, sizeof(g_fumo_storage_mm->acl_download_update));
    strncpy(g_fumo_storage_mm->acl_download_update_pkg_url, acl_get_replace, sizeof(g_fumo_storage_mm->acl_download_update_pkg_url));
    strncpy(g_fumo_storage_mm->acl_state, acl_get, sizeof(g_fumo_storage_mm->acl_state));
    strncpy(g_fumo_storage_mm->acl_ext, acl_get, sizeof(g_fumo_storage_mm->acl_ext));
    strncpy(g_fumo_storage_mm->acl_ext_severity, acl_get_replace, sizeof(g_fumo_storage_mm->acl_ext_severity));
    strncpy(g_fumo_storage_mm->acl_ext_wifi_only, acl_get_replace, sizeof(g_fumo_storage_mm->acl_ext_wifi_only));
    strncpy(g_fumo_storage_mm->acl_ext_wifi_only_timer, acl_get_replace, sizeof(g_fumo_storage_mm->acl_ext_wifi_only_timer));

    DM_LOGI("FUMO fumo_storage_init: }");
    return MO_ERROR_NONE;
}

int fumo_storage_reset_wifi()
{
    DM_LOGI("FUMO fumo_storage_reset_wifi: {");
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;
    pthread_mutex_lock(&storage_wait_mutex);
    if( MO_ERROR_NONE != (rc=fumo_storage_open())) {
            DM_LOGE("FUMO: fumo_storage_reset_wifi fumo_storage_open rc=%d", rc);
    } else {
        g_fumo_storage_mm->wifi_only = 0;
        g_fumo_storage_mm->wifi_only_timer = 23;
    }

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_reset_wifi fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO fumo_storage_reset_wifi: }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return MO_ERROR_NONE;
}

int fumo_storage_rest_defer_time()
{
    DM_LOGI("FUMO fumo_storage_rest_defer_time: {");
    int rc = MO_ERROR_COMMAND_FAILED;
    int rcc = MO_ERROR_COMMAND_FAILED;
    pthread_mutex_lock(&storage_wait_mutex);
    if( MO_ERROR_NONE != (rc=fumo_storage_open())) {
        DM_LOGE("FUMO: fumo_storage_rest_defer_time fumo_storage_open rc=%d", rc);
    } else {
        g_fumo_storage_mm->automatic_update_enable = 0;
        g_fumo_storage_mm->defer_time = 0;
    }

    if(MO_ERROR_NONE != (rcc=fumo_storage_close()))
        DM_LOGE("FUMO: fumo_storage_rest_defer_time fumo_storage_close rcc=%d", rcc);

    DM_LOGI("FUMO fumo_storage_rest_defer_time: }");
    pthread_mutex_unlock(&storage_wait_mutex);
    return MO_ERROR_NONE;
}

int fumo_storage_open(void)
{
    struct stat sb;
    int need_to_init = 0;
    DM_LOGI("FUMO: fumo_storage_open {");

    memset(&sb, 0, sizeof(sb));

    if(-1 != g_fumo_storage_file ||
                (fumo_storage_t*)MAP_FAILED != g_fumo_storage_mm) {
        DM_LOGE("FUMO: fumo_storage_open file %s or opened or mapped, \
force close", MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE);
        fumo_storage_close();
    }

    DM_LOGI("FUMO: fumo_storage_open opening %s",
                MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE);

    if(-1 == (g_fumo_storage_file=open(MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE,
            O_CLOEXEC|O_CREAT|O_SYNC|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP))) {
        DM_LOGE("FUMO: fumo_storage_open error [%s] on open %s",
            strerror(errno), MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE);
        return MO_ERROR_NOT_FOUND;
    }

    if(-1 == fstat(g_fumo_storage_file, &sb)) {
        DM_LOGE("FUMO: fumo_storage_open error [%s] on fstat %s",
            strerror(errno), MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE);
        return MO_ERROR_NOT_ALLOWED;
    }
    if(sb.st_size != sizeof(fumo_storage_t)) {
        DM_LOGE("FUMO: fumo_storage_open %s size is %jd, however %ld is read",
            MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE, (intmax_t)sb.st_size,
            (long int)sizeof(fumo_storage_t));
        if(0 != ftruncate(g_fumo_storage_file, sizeof(fumo_storage_t))) {
            DM_LOGE("FUMO: fumo_storage_open error [%s] on ftruncate",
                strerror(errno));
            return MO_ERROR_DEVICE_FULL;
        }
        need_to_init = 1;
    }

    g_fumo_storage_mm = (fumo_storage_t*)mmap(NULL, sizeof(fumo_storage_t),
        PROT_READ|PROT_WRITE, MAP_SHARED, g_fumo_storage_file, 0);

    if((fumo_storage_t*)MAP_FAILED == g_fumo_storage_mm) {
        DM_LOGE("FUMO: fumo_storage_open error [%s] on mmap",
            strerror(errno));
        return MO_ERROR_DEVICE_FULL;
    }

    if(need_to_init) { /** need to set values to default state */
        DM_LOGI("FUMO: fumo_storage_open: set values to default");
        fumo_storage_init();
    }

    DM_LOGI("FUMO: fumo_storage_open }");

    return MO_ERROR_NONE;
};

int fumo_storage_close(void)
{
    DM_LOGI("FUMO: fumo_storage_close {");

    if((fumo_storage_t*)MAP_FAILED != g_fumo_storage_mm) {
        if(-1 == munmap(g_fumo_storage_mm, sizeof(fumo_storage_t))) {
            DM_LOGE("FUMO: fumo_storage_close error [%s] on munmap",
                strerror(errno));
        }
        g_fumo_storage_mm = (fumo_storage_t*)MAP_FAILED;
    }

    if(-1 != g_fumo_storage_file) {
        if(0 != fsync(g_fumo_storage_file))
            DM_LOGE("FUMO: fumo storage sync error [%s]", strerror(errno));
        if(0 != close(g_fumo_storage_file) ) {
            DM_LOGE("FUMO: fumo_storage_close error [%s] on close",
                strerror(errno));
        }
        g_fumo_storage_file = -1;
    }

    DM_LOGI("FUMO: fumo_storage_close }");

    return MO_ERROR_NONE;
}

int fumo_storage_set_string_value(int offset, size_t max_len,
    const char *value, size_t value_len)
{
    int len = (value_len > max_len ? max_len-1 : value_len);
    DM_LOGI("FUMO fumo_storage_set_string_value: [%s]", value);
    pthread_mutex_lock(&storage_wait_mutex);
    if(MO_ERROR_NONE != fumo_storage_open()) {
        pthread_mutex_unlock(&storage_wait_mutex);
        return MO_ERROR_COMMAND_FAILED;
    }
    if(value){
        memcpy((char*)g_fumo_storage_mm + offset, value, len);
        ((char*)g_fumo_storage_mm)[offset + len] = '\0';
    } else {
        ((char*)g_fumo_storage_mm)[offset + 0] = '\0';
    }
    fumo_storage_close();
    pthread_mutex_unlock(&storage_wait_mutex);
    return MO_ERROR_NONE;
}

int fumo_storage_get_string_value(int offset, size_t max_len, char **value)
{
    pthread_mutex_lock(&storage_wait_mutex);
    if(MO_ERROR_NONE != fumo_storage_open()) {
        pthread_mutex_unlock(&storage_wait_mutex);
        return MO_ERROR_COMMAND_FAILED;
    }
    if(NULL == (*value=strndup((char*)g_fumo_storage_mm + offset, max_len))) {
        DM_LOGI("FUMO fumo_storage_get_string_value: out of memory");
        fumo_storage_close();
        pthread_mutex_unlock(&storage_wait_mutex);
        return MO_ERROR_DEVICE_FULL;
    }
    DM_LOGI("FUMO fumo_storage_get_string_value: [%s]", *value);
    fumo_storage_close();
    pthread_mutex_unlock(&storage_wait_mutex);
    return MO_ERROR_NONE;
}

int fumo_storage_set_int_value(int offset, int value)
{
    DM_LOGI("FUMO fumo_storage_set_int_value: %d", value);
    if(MO_ERROR_NONE != fumo_storage_open())
        return MO_ERROR_COMMAND_FAILED;
    *(int*)((char*)g_fumo_storage_mm + offset) = value;
    fumo_storage_close();
    return MO_ERROR_NONE;
}

int fumo_storage_get_int_value(int offset, int *value)
{
    pthread_mutex_lock(&storage_wait_mutex);
    if(MO_ERROR_NONE != fumo_storage_open()) {
        pthread_mutex_unlock(&storage_wait_mutex);
        return MO_ERROR_COMMAND_FAILED;
    }
    *value = *(int*)((char*)g_fumo_storage_mm + offset);
    DM_LOGI("FUMO fumo_storage_get_int_value: %lld", (long long)*value);
    fumo_storage_close();
    pthread_mutex_unlock(&storage_wait_mutex);
    return MO_ERROR_NONE;
}

int fumo_storage_set_time_value(int offset, __time_t value)
{
    pthread_mutex_lock(&storage_wait_mutex);
    DM_LOGI("FUMO fumo_storage_set_time_value: %lld",(long long) value);
    if(MO_ERROR_NONE != fumo_storage_open()) {
        pthread_mutex_unlock(&storage_wait_mutex);
        return MO_ERROR_COMMAND_FAILED;
    }
    *(__time_t*)((char*)g_fumo_storage_mm + offset) = value;
    fumo_storage_close();
    pthread_mutex_unlock(&storage_wait_mutex);
    return MO_ERROR_NONE;
}

int fumo_storage_get_time_value(int offset, __time_t *value)
{
    pthread_mutex_lock(&storage_wait_mutex);
    if(MO_ERROR_NONE != fumo_storage_open()) {
        pthread_mutex_unlock(&storage_wait_mutex);
        return MO_ERROR_COMMAND_FAILED;
    }
    *value = *(__time_t*)((char*)g_fumo_storage_mm + offset);
    DM_LOGI("FUMO fumo_storage_get_time_value: %lld", (long long)value);
    fumo_storage_close();
    pthread_mutex_unlock(&storage_wait_mutex);
    return MO_ERROR_NONE;
}

int fumo_storage_set_long_value(int offset, unsigned long value)
{
    DM_LOGI("FUMO fumo_storage_set_long_value: %lu", value);
    pthread_mutex_lock(&storage_wait_mutex);
    if(MO_ERROR_NONE != fumo_storage_open()) {
        pthread_mutex_unlock(&storage_wait_mutex);
        return MO_ERROR_COMMAND_FAILED;
    }
    *(unsigned long*)((char*)g_fumo_storage_mm + offset) = value;
    fumo_storage_close();
    pthread_mutex_unlock(&storage_wait_mutex);
    return MO_ERROR_NONE;
}

int fumo_storage_get_long_value(int offset, unsigned long *value)
{
    pthread_mutex_lock(&storage_wait_mutex);
    if(MO_ERROR_NONE != fumo_storage_open()) {
         pthread_mutex_unlock(&storage_wait_mutex);
        return MO_ERROR_COMMAND_FAILED;
    }
    *value = *(unsigned long*)((char*)g_fumo_storage_mm + offset);
    DM_LOGI("FUMO fumo_storage_get_long_value: %lu", *value);
     pthread_mutex_unlock(&storage_wait_mutex);
    fumo_storage_close();
    return MO_ERROR_NONE;
}

int get_version_from_name_pkg(char* fwname)
{
    char *indxDash = NULL;
    if(NULL == (indxDash=strrchr(fwname, (int)'_')) ||
            ++indxDash >= fwname+strlen(fwname))
        indxDash = fwname;
    DM_LOGI("FUMO get_version_from_name_pkg %s", indxDash);
    strncpy(g_fumo_storage_mm->pkg_version, indxDash,
                    sizeof(g_fumo_storage_mm->pkg_version));
    return MO_ERROR_NONE;
}