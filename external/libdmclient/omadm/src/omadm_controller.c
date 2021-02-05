/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * This component manages OMADM session from start to finish. It uses OMADM
 * Interface Handler to send/receive packages to/from OMADM server. It uses MO
 * plugins to process commands from the server.
 * Implements API:
 * omadm_controller_start_session(void *request)
 * omadm_controller_interrupt_session()
 * Uses API:
 * omadm_interface_handler_post_package(dmclt_buffer_t* packet,
 *     dmclt_buffer_t* reply, bool format_type, char *username, char *password)
 */
#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dm_logger.h"
#include "mo_error.h"
#include "mo_omadmtree.h"
#include "omadm_controller.h"
#include "omadm_interface_handler.h"
#include "omadm_event_handler.h"
#include "net_manager.h"

#include <hardware_legacy/power.h>

#define DMACC_MO_URN "urn:oma:mo:oma-dm-dmacc:1.0"
#define PARTIAL_WAKE_LOCK 1
#define DMCLIENT_WAKE_LOCK_NAME "omadm"

/// Maximum number of plugins which cab be loaded
#define MAX_PLUGIN 16

/// Size of array for logging into logcat/syslog
#define DM_BUFFER_SIZE 512

#ifdef ANDROID
#define USER_REACTION_TIMEOUT 100
static pthread_mutex_t gui_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gui_cond  = PTHREAD_COND_INITIALIZER;
#define GUI_LOCK()      pthread_mutex_lock(&gui_lock)
#define GUI_UNLOCK()    pthread_mutex_unlock(&gui_lock)
#define GUI_SIGNAL()    pthread_cond_signal(&gui_cond)
static pal_omadm_controller_gui_user_reply_param* pControllerGuiUserRP = NULL;
static void* palHandle = NULL;
static int (*pal_omadm_controller_gui_create_message_fn)(pal_omadm_controller_gui_message_descriptor*) = NULL;
#endif

#define USERREQUEST_TYPE   "org.openmobilealliance.dm.firmwareupdate.userrequest"
#define USERREQUEST_FORMAT "b64"
#define USERREQUEST_DATA   "Y2hlY2tDb25maWd1cmF0aW9u"
#define DOWNLOAD_FLAG_PATH_FILE "/data/vendor/verizon/dmclient/data/download_flag"
#define NETWORK_UP_FLAG_PATH_FILE "/data/vendor/verizon/dmclient/data/network_up_flag"

/// Pointer to the internal session structure
dmclt_session g_session = NULL;

/// Flag for interrupting excecute session
bool is_session_interrupted = false;

/// Array for handles of the loaded plugins
void * pluginHandles[MAX_PLUGIN];

/// Default session environment
omadm_env_t env;

/// Event handler callbacks
void * cbData = NULL; /// data was got from initFunc result

/// Flag which describe state of WiFi data connection at the start session moment
bool is_init_wifi_connected = false;

bool check_download_flag()
{
	if ( 0 != access(DOWNLOAD_FLAG_PATH_FILE, F_OK))
	{
		return false;
	}

	return true;
}

bool check_network_up_flag()
{
	if ( 0 != access(NETWORK_UP_FLAG_PATH_FILE, F_OK))
	{
		return false;
	}

	return true;
}

#ifdef ANDROID
/**
 * Callback for return user reaction data
 * @brief pal_gui_callback
 * @param[in] data user reaction data
 */
static void pal_gui_callback(void* data)
{
    DM_LOGI("OMADM_C: pal_gui_callback caller info: pid %d", getpid());
    GUI_LOCK();
    pControllerGuiUserRP = (pal_omadm_controller_gui_user_reply_param*)data;
    GUI_SIGNAL();
    GUI_UNLOCK();
}

/**
 * Prepare user data from PAL
 * @brief get_gui_data_from_pal
 * @param[in] alertData the alert data from a received syncml package
 * @param[in] userReply the user responds
 * @param[in] code the status of user responds
 */
static void get_gui_data_from_pal(const dmclt_ui_t * alertData, char * userReply, int* code)
{
    int pal_status = RESULT_ERROR;
    *code = OMADM_SYNCML_ERROR_SUCCESS;
    if (pal_omadm_controller_gui_create_message_fn == NULL) {
        DM_LOGE("OMADM_C: pal_omadm_controller_gui_create_message_fn pointer was not initialized");
        return;
    }
    pal_omadm_controller_gui_message_descriptor *pmds =
        (pal_omadm_controller_gui_message_descriptor*)malloc(sizeof(pal_omadm_controller_gui_message_descriptor));
    if( pmds == NULL ) {
        DM_LOGE("OMADM_C: get_gui_data_from_pal - memory allocation error");
        return;
    }
    memset(pmds, 0, sizeof(pal_omadm_controller_gui_message_descriptor));
    pmds->type = alertData->type;
    pmds->min_disp = alertData->min_disp;
    pmds->max_disp = alertData->max_disp;
    pmds->max_resp_len = alertData->max_resp_len;
    pmds->input_type = alertData->input_type;
    pmds->echo_type = alertData->echo_type;
    pmds->disp_msg = alertData->disp_msg;
    pmds->dflt_resp = alertData->dflt_resp;
    pmds->choices = (const char**)alertData->choices;
    pmds->user_reaction = pal_gui_callback;
    pal_status = pal_omadm_controller_gui_create_message_fn(pmds);
    free(pmds);
    pmds = NULL;

    /// In case when we have display type dialogs we do not care about handle pal callbacks
    if (alertData->type == DMCLT_UI_TYPE_DISPLAY ||
        alertData->type == DMCLT_UI_TYPE_DISPLAY_UIE) {
        return;
    }

    if (pal_status == RESULT_SUCCESS) {
        GUI_LOCK();
        DM_LOGI("OMADM_C: get_gui_data_from_pal start waiting Controller GUI PAL callback");
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += USER_REACTION_TIMEOUT;
        pthread_cond_timedwait(&gui_cond, &gui_lock, &timeout);
        GUI_UNLOCK();

        if (pControllerGuiUserRP != NULL) {
            switch (pControllerGuiUserRP->type) {
                case DMCLT_UI_TYPE_DISPLAY:
                    break;
                case DMCLT_UI_TYPE_CONFIRM:
                    if (pControllerGuiUserRP->button_id != CONFIRM_ACTION) {
                        *code = OMADM_SYNCML_ERROR_NOT_MODIFIED;
                    }
                    break;
                case DMCLT_UI_TYPE_USER_INPUT:
                    if (strlen(pControllerGuiUserRP->input_text) == 0) {
                        *code = OMADM_SYNCML_ERROR_OPERATION_CANCELED;
                    } else {
                        strncpy(userReply, pControllerGuiUserRP->input_text, alertData->max_resp_len);
                    }
                    break;
                case DMCLT_UI_TYPE_USER_CHOICE:
                    if (pControllerGuiUserRP->selected_choices_count == 0) {
                        *code = OMADM_SYNCML_ERROR_OPERATION_CANCELED;
                    } else {
                        strncpy(userReply, pControllerGuiUserRP->selected_choices[0], alertData->max_resp_len);
                    }
                    break;
                case DMCLT_UI_TYPE_USER_MULTICHOICE:
                     /// \todo add processing
                    break;
                default:
                    *code = OMADM_SYNCML_ERROR_COMMAND_FAILED;
                    break;
            }

            if (pControllerGuiUserRP->input_text) {
                free(pControllerGuiUserRP->input_text);
                pControllerGuiUserRP->input_text = NULL;
            }
            if (pControllerGuiUserRP->selected_choices) {
                int i = 0;
                do {
                    free(pControllerGuiUserRP->selected_choices[i]);
                    pControllerGuiUserRP->selected_choices[i] = NULL;
                    i++;
                } while(i < pControllerGuiUserRP->selected_choices_count);
                free(pControllerGuiUserRP->selected_choices);
            }
            free(pControllerGuiUserRP);
            pControllerGuiUserRP = NULL;
        } else {
            *code = OMADM_SYNCML_ERROR_COMMAND_FAILED;
        }
    } else {
        *code = OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("OMADM_C: get_gui_data_from_pal return data: return_code = %d, userReply = %s",
        *code, userReply);
}
#endif

/**
 * Callback function to process alert messages
 * @brief gui_callback
 * @param[in] userData  the user data
 * @param[in] alertData the alert data from a received syncml package
 * @param[in] userReply the user responds
 * @return status status code depending on user responds
 */
static int gui_callback(void * userData, const dmclt_ui_t * alertData, char * userReply)
{
    DM_LOGI("OMADM_C: gui_callback caller info: pid %d", getpid());
    int return_code = OMADM_SYNCML_ERROR_SUCCESS;
#ifndef ANDROID
    /// For all devices without UI all confirmation requests are auto-approved
    return return_code;
#else
    get_gui_data_from_pal(alertData, userReply, &return_code);
    return return_code;
#endif
}

/**
 * Callback for sending event
 * @brief event_handler_callback
 * @param[in] event event passed from MO to DM-client
 * @return 0 if success
 */
int event_handler_callback(omadm_mo_event_t *event)
{
    int status = 0;
    DM_LOGI("OMADM_C: event_handler_callback");
    if(event == NULL) {
        DM_LOGI("OMADM_C: event_handler_callback - invalid event!");
        return RESULT_ERROR_INVALID_ARGS;
    }
    alert_t * alert = malloc(sizeof(alert_t ));
    if( alert == NULL ) {
        DM_LOGI("OMADM_C: event_handler_callback - memory allocation error");
        return RESULT_MEMORY_ERROR;
    }
    alert->mo = strdup("FUMO");
    alert->alert_data = (char*)event;

    status = omadm_event_handler_process_alert(alert);
    return status;
}

/**
 * Print out the data buffer
 * @brief output_buffer
 * @param fd      file descriptor
 * @param isWbxml format type
 * @param buffer  data buffer for printing
 */
void output_buffer(bool isWbxml, dmclt_buffer_t buffer)
{
    (void)isWbxml;
    (void)buffer;
    char array[DM_BUFFER_SIZE];
    DM_LOGI("OMADM_C: output_buffer isWbxml = %d", isWbxml);
    memset(array, 0, sizeof(char) * DM_BUFFER_SIZE);
    if (isWbxml) {
//        unsigned char array[16];

//        i = 0;
//        while (i < buffer.length) {
//            int j;
////            DM_LOGI("OMADM_C:   ");

//            memcpy(array, buffer.data + i, 16);

//            for (j = 0; j < 16 && i + j < buffer.length; j++) {
////                DM_LOGI("OMADM_C: %02X ", array[j]);
//            }
//            while (j < 16) {
//                DM_LOGI("OMADM_C:    ");
//                j++;
//            }
//            DM_LOGI(fd, "  ");
//            for (j = 0; j < 16 && i + j < buffer.length; j++) {
//                if (isprint(array[j]))
//                    DM_LOGI("OMADM_C: %c ", array[j]);
//                else
//                    DM_LOGI("OMADM_C: . ");
//            }
//            DM_LOGI("OMADM_C: ");

//            i += 16;
//        }
    } else {
        int i, j, cnt = 0, tab = -2;
        for (i = 0; i < buffer.length; i++) {
            if (buffer.data[i] == '<') {
                if (i + 1 < buffer.length && buffer.data[i + 1] == '/') {
                    tab--;
                    if (i != 0 && buffer.data[i - 1] == '>') {
                        DM_LOGI("OMADM_C: %s", array);
                        memset(array, 0, sizeof(char) * DM_BUFFER_SIZE);
                        cnt = 0;
                        for (j = 0; j < tab * 4; j++)
                            cnt += sprintf(array+cnt, " ");
                    }
                } else {
                    if (i != 0 && buffer.data[i - 1] == '>') {
                        DM_LOGI("OMADM_C: %s", array);
                        memset(array, 0, sizeof(char) * DM_BUFFER_SIZE);
                        cnt = 0;
                        for (j = 0; j < tab * 4; j++)
                            cnt += sprintf(array+cnt, " ");
                    }
                    tab++;
                }
            }
            if (cnt < DM_BUFFER_SIZE) {
                cnt += sprintf(array+cnt, "%c", buffer.data[i]);
            }
        }
    }
    DM_LOGI("OMADM_C: %s", array);
}

/// \todo Temporary solution for restarting DM session
dmclt_err_t restart_session() {
    dmclt_err_t err = omadm_event_handler_process_event(START_AUTOSESSION, NULL);
    DM_LOGI("OMADM_C: call Event Handler process event. res = %d", err);
    return err;
}

/// \todo Temporary solution for emulation of SU Cancel request
dmclt_err_t emulate_su_cancel() {
    dmclt_err_t err = omadm_event_handler_process_event(SU_CANCEL, NULL);
    DM_LOGI("OMADM_C: call Event Handler process event. res = %d", err);
    return err;
}

dmclt_err_t user_update_session() {
    dmclt_err_t err = omadm_event_handler_process_event(USER_SYS_UPDATE, NULL);
    DM_LOGI("OMADM_C: call Event Handler process event. res = %d", err);
    return err;
}

int dmclient_wake_lock_acquire()
{
	acquire_wake_lock(PARTIAL_WAKE_LOCK, DMCLIENT_WAKE_LOCK_NAME);
	return 0;
}

int dmclient_wake_lock_release()
{
	release_wake_lock(DMCLIENT_WAKE_LOCK_NAME);
	return 0;
}
#define WAKE_LOCK "/sys/power/wake_lock"
#define WAKE_UNLOCK "/sys/power/wake_unlock"

/**
 * This function gets the new SYNCML package and sends it to the server
 * via the interface handler
 * @brief execute_session
 * @param[in] request the parameters required to the session
 * @return dm session status
 */
static dmclt_err_t execute_session(omadm_env_t *env)
{
    is_init_wifi_connected = netm_is_wifi_connected();
    DM_LOGI("OMADM_C: execute_session() with WiFi status [%d]",
        (int)is_init_wifi_connected);
    dmclt_buffer_t packet; // data buffer to server
    dmclt_buffer_t reply;  // reply buffer from server
    dmclt_err_t err = DMCLT_ERR_INTERNAL;
    if (g_session != NULL) {
        memset(&packet, 0, sizeof(dmclt_buffer_t));
        memset(&reply, 0, sizeof(dmclt_buffer_t));
        char * username = NULL;
        char * password = NULL;
        do {
            if (is_session_interrupted) {
                is_session_interrupted = false;
                DM_LOGI("OMADM_C: Session is interrupted");
                return err;
            }

            err = omadmclient_get_next_packet(g_session, &packet);
            //DM_LOGI("OMADM_C: omadmclient_get_next_packet err = %d\n", err);
            if (DMCLT_ERR_NONE == err) {
                DM_LOGI("OMADM_C: PACKET to uri = %s ", packet.uri);
                output_buffer(env->format_type, packet);
                if (is_init_wifi_connected &&
                        netm_is_wifi_connected() != is_init_wifi_connected) {
                    err = DMCLT_ERR_ABORT;
                } else {
//add barret only for set APN start
                	if(!netm_is_wifi_connected()){
                		void * pal_handle = NULL;
                        pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
                        if(pal_handle == NULL) {
                        	DM_LOGE("NM: can't open PAL lib");
                            // dmclient shall continue work
                        } else {
                        	int (*testFunc)() = dlsym(pal_handle, "get_network_status");
                        	int net_status=0;

                            if(testFunc == NULL) {
                                DM_LOGE("NM: testFunc not found!");
                                // dmclient shall continue work
                            } else {
                            	net_status= testFunc();
                                DM_LOGD("NM: net_status status is [%d]", net_status);
                            }
                            if(net_status==3){
                            	netm_down_admin_network();
                                   DM_LOGD("NM: ydc test off -> ON admin apn - status");
                                    netm_up_admin_network();
                                    if(testFunc()==3){
                                    netm_is_network_ready_lock();
                                    }
                            }
                        }
                    }
//add barret only for set APN end
                    err = omadm_interface_handler_post_package(&packet, &reply,
                            env->format_type, username, password);
                }
                DM_LOGI("OMADM_C: REPLY from \"%s\": err = %d : %ld bytes",
                        packet.uri, err, reply.length);
                omadmclient_clean_buffer(&packet);
                if (DMCLT_ERR_NONE == err) {
                    output_buffer(env->format_type, reply);
                    err = omadmclient_process_reply(g_session, &reply);
                    omadmclient_clean_buffer(&reply);
                    DM_LOGI("OMADM_C: Reply processed. res = %d", err);
                }
            }
        } while (DMCLT_ERR_NONE == err || check_download_flag());
        DM_LOGI("OMADM_C: SESSION FINISHED");
    } else {
        DM_LOGI("OMADM_C: Session failed");
    }
    return err;
}

/**
 * Load eternal plugins at start up from the plugins directory
 * @brief omadm_controller_load_eternal_plugins
 */
void omadm_controller_load_eternal_plugins()
{
    int i = 0;
    DIR *folderP;
    struct dirent *fileP;

    DM_LOGD("llload_eternal_plugins MO_INSTALL_DIR = %s", MO_INSTALL_DIR);
    folderP = opendir(MO_INSTALL_DIR);
    if(folderP == NULL) {
        DM_LOGE("Could not open plugin dir' %s errno = %d", MO_INSTALL_DIR, errno);
        return;
    }
    while ((fileP = readdir(folderP)) && (i < MAX_PLUGIN)) {
        DM_LOGD("MO dir entry: %d - %s [%d] %d",
                fileP->d_ino, fileP->d_name, fileP->d_type, fileP->d_reclen);
        if (DT_REG == fileP->d_type) {
            char * filename;
            void * handle = NULL;
            omadm_mo_interface_t * (*getMoIfaceF)();
            int (*moRegEventH)(void *, omadm_mo_ft_event_handler);

            filename = (char *) malloc(
                        strlen(MO_INSTALL_DIR) + 1 + strlen(fileP->d_name) + 1);
            if(!filename) {
                DM_LOGE("OMADM_C: memory allocation error!");
                i++;
                continue;
            }
            sprintf(filename, "%s", MO_INSTALL_DIR);
            strcat(filename, "/");
            strcat(filename, fileP->d_name);
            handle = dlopen(filename, RTLD_LAZY);
            if (handle) {
                getMoIfaceF = dlsym(handle, "omadm_get_mo_interface");
                if (getMoIfaceF) {
                    omadm_mo_interface_t * moInterfaceP = NULL;
                    moInterfaceP = getMoIfaceF();
                    if (moInterfaceP && (moInterfaceP->base_uri)) {
                        // Registering event handler into MO plugin
                        moRegEventH = dlsym(handle, "omadm_mo_register_event_handler");
                        if(moRegEventH) {
                            // FUMO should be loaded by default
                            DM_LOGI("OMADM_C: load at start up: plugin %s", filename);
                            moInterfaceP->initFunc(NULL);
                            int res = moRegEventH(NULL, event_handler_callback);
                            if (res != 0) {
                                DM_LOGI("OMADM_C: Can't register event handler %d", res);
                            }
                            i++;
                            handle = NULL;
                        }
                    }
                }
                if (handle) {
                    dlclose(handle);
                    handle = NULL;
                }
            }
            free(filename);
        }
    }
    DM_LOGI("OMADM_C: %d plugins were loaded", i);
    closedir(folderP);
}

/**
 * Load plugins from the plugins directory
 * @brief load_plugins
 * @param[in] session the pointer to internal session structure
 */
static void load_plugins(dmclt_session session)
{
    DIR *folderP;
    int i;
    DM_LOGD("load_plugins MO_INSTALL_DIR = %s",MO_INSTALL_DIR);
    memset(pluginHandles, 0, MAX_PLUGIN * sizeof(void*));
    i = 0;
    folderP = opendir(MO_INSTALL_DIR);
    if(folderP == NULL) {
        DM_LOGE("Could not open plugin dir' %s", MO_INSTALL_DIR);
        return;
    }
    struct dirent *fileP;
    while ((fileP = readdir(folderP)) && (i < MAX_PLUGIN)) {
        DM_LOGD("MO dir entry: %d - %s [%d] %d",
                fileP->d_ino, fileP->d_name, fileP->d_type, fileP->d_reclen);
        if (DT_REG == fileP->d_type) {
            char * filename;
            void * handle = NULL;
            omadm_mo_interface_t * (*getMoIfaceF)();

            filename = (char *) malloc(
                        strlen(MO_INSTALL_DIR) + 1 + strlen(fileP->d_name) + 1);
            if(!filename) {
                DM_LOGE("OMADM_C: memory allocation error!");
                i++;
                continue;
            }
            sprintf(filename, "%s", MO_INSTALL_DIR);
            strcat(filename, "/");
            strcat(filename, fileP->d_name);
            handle = dlopen(filename, RTLD_LAZY);
            if (handle) {
                getMoIfaceF = dlsym(handle, "omadm_get_mo_interface");
                if (getMoIfaceF) {
                    omadm_mo_interface_t * moInterfaceP = NULL;
                    moInterfaceP = getMoIfaceF();
                    if (moInterfaceP && (moInterfaceP->base_uri)) {
                        if (DMCLT_ERR_NONE
                                 == omadmclient_session_add_mo(session,
                                                         moInterfaceP)) {
                            DM_LOGI("OMADM_C: load plugin %s", filename);
                            // store handle
                            pluginHandles[i] = handle;
                            // Registering event handler into MO plugin
                            int (*moRegEventH)(void*, omadm_mo_ft_event_handler);
                            moRegEventH = dlsym(handle,
                                     "omadm_mo_register_event_handler");
                            if(moRegEventH) {
                                int res = moRegEventH(NULL, event_handler_callback);
                                if (res != 0) {
                                    DM_LOGI("OMADM_C: Can't register event handler %d",
                                            res);
                                }
                            }
                            i++;
                            handle = NULL;
                        } else {
                            DM_LOGI("OMADM_C: Can't load plugin %s", filename);
                            free(moInterfaceP);
                        }
                    }
                }
                if (handle)
                    dlclose(handle);
            }
            free(filename);
        }
    }
    DM_LOGI("OMADM_C: %d plugins were added", i);
    closedir(folderP);
}

/**
 * Unload plugins from handling
 * @brief unload_plugins
 */
static void unload_plugins()
{
    int i = 0;
    while ((i < MAX_PLUGIN) && (pluginHandles[i] != 0)) {
        // Unregistering event handler
        int (*moUnregEventH)(void *, omadm_mo_ft_event_handler);
        moUnregEventH = dlsym(pluginHandles[i],
                "omadm_mo_unregister_event_handler");
        if(moUnregEventH) {
            // FUMO should not be unloaded
            i++;
            continue;
        }
        dlclose(pluginHandles[i]);
        pluginHandles[i] = NULL;
        DM_LOGI("OMADM_C: unload plugin: %d",i);
        i++;
    }
}

/**
 * Setup environment for OMADM session
 * @brief setup_session
 * @param envP session environment
 * @return status DMCLT_ERR_NONE if success, DMCLT_ERR_INTERNAL otherwise
 */
dmclt_err_t setup_session(omadm_env_t *envP) {
    dmclt_err_t status = DMCLT_ERR_INTERNAL;
    dmtree_node_t node;
    internals_t * internP = (internals_t *) g_session;
    memset(&env, 0, sizeof(omadm_env_t));
    DM_LOGI("OMADM_C: setup_session envP = %p internP = %p\n",envP,internP);
    if (NULL == envP && NULL != internP) {
        memset(&node, 0, sizeof(dmtree_node_t));
        env.format_type = false;
        node.uri = strdup("./DMAcc/ServerID");
        momgr_get_value(internP->dmtreeH->MOs, &node);
        if (NULL != node.data_buffer) {
            env.server_id = dmtree_node_as_string(&node);
        }
        dmtree_node_clean(&node, true);
        env.session_id = 1;
    } else {
        env.format_type = envP->format_type;
        env.server_id = envP->server_id;
        env.session_id = envP->session_id;
    }
    if (NULL != env.server_id && 0 != env.session_id) {
        status = DMCLT_ERR_NONE;
    }
    return status;
}

/**
 * Cancels ongoing OMADM session
 *  @brief close_session
 */
void close_session()
{
    DM_LOGI("OMADM_C: close session");
    if (g_session != NULL) {
        // close session
        omadmclient_session_close(g_session);
        free(g_session);
        g_session = NULL;
        free(env.server_id);
        memset(&env, 0, sizeof(omadm_env_t));
        unload_plugins();
        omadm_interface_handler_end_session();
    } else {
        DM_LOGI("OMADM_C: session not started");
    }
#ifdef ANDROID
    if (palHandle)
    {
        dlclose(palHandle);
        palHandle = NULL;
    }
#endif
}

/**
 * Starts a new OMADM session
 *  @brief start_session
 *  @return dm session status: DMCLT_ERR_NONE if success
 *                             DMCLT_ERR_INTERNAL otherwise
 */
dmclt_err_t start_session()
{
    dmclt_err_t status = DMCLT_ERR_NONE;
    if (NULL == env.server_id) {
        status = setup_session(NULL);
    }
    if (DMCLT_ERR_NONE == status) {
        DM_LOGI("OMADM_C: format_type = %d server_id = %s session_id = %d",
                (int) env.format_type, env.server_id, env.session_id);
        status = omadmclient_session_start(g_session, env.server_id,
                env.session_id);
    }
    return status;
}

/**
 * Initialization of Omadm Controller internal variables
 *  @brief omadm_controller_init
 */
bool omadm_controller_init()
{
#ifdef ANDROID
    palHandle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    if (palHandle == NULL) {
        DM_LOGE("OMADM_C: ERR palHandle == NULL");
    }
    pal_omadm_controller_gui_create_message_fn = dlsym(palHandle,
        "pal_omadm_controller_gui_create_message");
    if (pal_omadm_controller_gui_create_message_fn == NULL) {
        dlclose(palHandle);
        DM_LOGE("OMADM_C: ERR pal_omadm_controller_gui_create_message symbol not found");
    }
#endif
    return true;
}

dmclt_err_t omadm_controller_handle_request(void * request)
{
    dmclt_err_t status = DMCLT_ERR_NONE;
    DM_LOGI("OMADM_C:hh HANDLE session() called");
    omadm_request_t * omadm_request = (omadm_request_t *) request;
    dmclient_wake_lock_acquire();

    netm_up_admin_network();
	DM_LOGI("handle_request omadm_request->type = %d\n", omadm_request->type);
#ifdef ANDROID
    if (USER_SYS_UPDATE == omadm_request->type && !netm_is_network_ready()) {
        omadm_controller_show_ui_dialog(DMCLT_UI_TYPE_DISPLAY_UIE);
        DM_LOGI("OMADM_C: handle_request() show connection fail UI dialog");
        return DMCLT_ERR_NONE;
    }
#endif
//    netm_is_network_ready_lock();
    DM_LOGD("Network is ready");

    if (omadm_request == NULL) {
        DM_LOGE("OMADM_C: Invalid request paramener!");
        return DMCLT_ERR_USAGE;
    } else {
        if (g_session != NULL) {
            DM_LOGI("OMADM_C: Initialization failed. Session already going");
            return DMCLT_ERR_INTERNAL;
        }

        // init session
        g_session = omadmclient_session_init(env.format_type);
        if (g_session == NULL) {
            DM_LOGI("OMADM_C: Initialization failed");
            return DMCLT_ERR_INTERNAL;
        }
        status = omadmclient_set_UI_callback(g_session, gui_callback, NULL);
        if (DMCLT_ERR_NONE != status) {
            DM_LOGI("OMADM_C: Initialization failed: %d", status);
            return DMCLT_ERR_INTERNAL;
        }
        // load plugins
        load_plugins(g_session);
        status = omadm_controller_start_session(omadm_request);
        DM_LOGD("OMADM_C: handle request: Start session has executed with status: %d",
                status);

#ifdef ANDROID
        /**
         * For User initiated session we should show gui display type controller alert
         * according to Verizon_OTADM_Reference_Client 7.4 requirements
         * DMCLT_ERR_INTERNAL status defines network problems during the session
         */
        if (DMCLT_ERR_INTERNAL == status && USER_SYS_UPDATE == omadm_request->type) {
            omadm_controller_show_ui_dialog(DMCLT_UI_TYPE_DISPLAY_UIE);
        }
#endif

        if (DMCLT_ERR_ABORT == status && is_init_wifi_connected &&
                netm_is_wifi_connected() != is_init_wifi_connected) {
            netm_up_admin_network();
            if(netm_is_network_ready()) {
                omadm_request_t omadm_abort_request;
                omadm_abort_request.alert = malloc(sizeof(alert_t));
                if (omadm_abort_request.alert != NULL) {
                    memset(omadm_abort_request.alert,0,sizeof(alert_t));
                    omadm_abort_request.alert->alert_data =
                            strdup(OMADM_ALERT_SESSION_ABORT);
                }
                omadm_abort_request.package0 = NULL;
                omadm_abort_request.type = SESSION_ALERT;
                status = omadm_controller_start_session(&omadm_abort_request);
                DM_LOGD("OMADM_C: handle request: Abort session has executed with status: %d",
                    status);
            }
        }
        is_init_wifi_connected = false;
    }
    // close session
    close_session();
    DM_LOGD("OMADM_C: handle request: Close session has executed");

    // release admin apn network interface
   	netm_down_admin_network();
	dmclient_wake_lock_release();

    return status;
}


dmclt_err_t omadm_controller_show_ui_dialog(dmclt_ui_type_t type)
{
    dmclt_err_t status = DMCLT_ERR_NONE;
    dmclt_ui_t *pclui = (dmclt_ui_t*)malloc(sizeof(dmclt_ui_t));
    if( pclui == NULL ) {
        DM_LOGE("OMADM_C: omadm_controller_show_ui_dialog - memory allocation error");
        status = DMCLT_ERR_MEMORY;
    } else {
        memset(pclui, 0, sizeof(dmclt_ui_t));
        pclui->type = type;
        gui_callback(NULL, pclui, NULL);
        free(pclui);
        pclui = NULL;
    }
    return status;
}


dmclt_err_t omadm_controller_start_session(omadm_request_t * omadm_request)
{
    dmclt_err_t status = DMCLT_ERR_NONE;
    DM_LOGI("OMADM_C: START session() called");

    if (PACKAGE0 == omadm_request->type) {
        DM_LOGD("OMADM_C: Start session from event: PACKAGE0");
        if (NULL != omadm_request->package0) {
            status = omadmclient_session_start_on_alert(g_session,
                    omadm_request->package0->buffer, omadm_request->package0->len,
                    0 /*char * flags*/, 0 /*int * body_offset*/);
        } else {
            DM_LOGI("OMADM_C: Start session failed: Package0 is NULL");
            status = DMCLT_ERR_USAGE;
        }
    } else if (USER_SYS_UPDATE == omadm_request->type) {
        DM_LOGD("OMADM_C: Start session from event: USER_SYS_UPDATE");
        status = start_session();
        // according to Verizon_OTADM_Reference_Client,section 25.2
        // when the end user selects Check for new software update,
        // the message MUST follow the Generic Alert format
        if (DMCLT_ERR_NONE == status) {
            dmclt_item_t * item = calloc(sizeof(dmclt_item_t),1);
            if(item == NULL)
                return MO_ERROR_DEVICE_FULL;
            item->format = USERREQUEST_FORMAT;
            item->type = USERREQUEST_TYPE;
            item->data = USERREQUEST_DATA;
            item->target = NULL;
            internals_t * internP = (internals_t *)g_session;
            if(internP && internP->account && internP->account->toServerCred &&
                    internP->account->toServerCred->name) {
                int imei_len = strlen("IMEI:");
                int name_len = strlen(internP->account->toServerCred->name);
                item->source = calloc(sizeof(char),name_len+imei_len+1);
                if(item->source == NULL) {
                    free(item);
                    return MO_ERROR_DEVICE_FULL;
                }
                strncpy(item->source,"IMEI:",imei_len);
                strncat(item->source,internP->account->toServerCred->name,
                        name_len);
            }
            status = omadmclient_add_generic_alert(g_session, NULL, item);

            DM_LOGI("OMADM_C: omadmclient_add_generic_alert res = %d",
                                                              status);
            if(item->source)
                free(item->source);
            if(item)
                free(item);
        }

    } else if (START_AUTOSESSION == omadm_request->type) {
        DM_LOGD("OMADM_C: Start session from event: START_AUTOSESSION");
        status = start_session();
    } else if (GENERIC_ALERT == omadm_request->type) {
        DM_LOGD("OMADM_C: Start session from event: GENERIC ALERT");
        omadm_mo_event_t * event = NULL;
        if(omadm_request->alert) {
            if(omadm_request->alert->alert_data) {
                event = (omadm_mo_event_t *)omadm_request->alert->alert_data;
                status = start_session();
                if (DMCLT_ERR_NONE == status) {
                    status = omadmclient_add_generic_alert(g_session,
                                               event ? event->correlator: 0,
                                               event ? &event->dmclt_item : 0);
                    DM_LOGI("OMADM_C: omadmclient_add_generic_alert res = %d",
                                                                      status);
                }
            } else
                DM_LOGI("OMADM_C: there are no data for generic alert- do nothing");
        } else
            DM_LOGI("OMADM_C: omadm_request->alert = NULL");
    } else if(SESSION_ALERT == omadm_request->type){
      char *code = NULL;
      DM_LOGD("OMADM_C: Start session from event: ALERT");
      if(omadm_request->alert) {
          if(omadm_request->alert->alert_data) {
              code  = omadm_request->alert->alert_data;
              status = start_session();
              if (DMCLT_ERR_NONE == status) {
                  status = omadmclient_add_alert(g_session, code);
                  DM_LOGI("OMADM_C: omadmclient_add_alert code = %s res = %d",
                          code, status);
              }
          } else
              DM_LOGI("OMADM_C: there are no data for alert - do nothing");
      } else
           DM_LOGI("OMADM_C: omadm_request->alert = NULL");
    } else if (SU_CANCEL == omadm_request->type) {
        DM_LOGD("OMADM_C: Start session from event: SU_CANCEL");
        ///\todo  call FUMO exec function
        int i = 0;
        if (NULL != omadm_request->package0) {
            DM_LOGD("OMADM_C: WAP PUSH SU Cancel processing");
            status = omadmclient_session_start_on_alert(g_session,
                    omadm_request->package0->buffer, omadm_request->package0->len,
                    0 /*char * flags*/, 0 /*int * body_offset*/);
            ///\todo for test purpose only, remove in a final release
            status = DMCLT_ERR_NONE;
        }
        while ((i < MAX_PLUGIN) && (pluginHandles[i] != 0) &&
                DMCLT_ERR_NONE == status){
            omadm_mo_interface_t * (*getMoIfaceF)();
            getMoIfaceF = dlsym(pluginHandles[i], "omadm_get_mo_interface");
            i++;
            if(!getMoIfaceF)
                continue;
            omadm_mo_interface_t * iface = getMoIfaceF();
            if( iface->execFunc == NULL) {
                DM_LOGE("OMADM_C: exec function isn't supported %d", i);
            } else {
                DM_LOGE("OMADM_C: exec function found %d",i);
                iface->execFunc("./ManagedObjects/FUMO/Cancel",/*const char * uri*/
                                0,/*const char * cmdData*/
                                0,/*const char * correlator*/
                                0/*void * data*/
                                );
            }
        }
    } else {
        DM_LOGI("OMADM_C: Start session failed: Unknown event");
        status = DMCLT_ERR_USAGE;
    }

    // Execute session
    if (DMCLT_ERR_NONE == status) {
        status = execute_session(&env);
        DM_LOGD("OMADM_C: Session has executed with status: %d", status);
    } else if (PACKAGE0 == omadm_request->type) {
        // \todo: This particular case will be reworked
        status = execute_session(&env);
        DM_LOGD("OMADM_C: Session has executed with status: %d", status);
    } else {
        DM_LOGI("OMADM_C: Session opening to \"%s\" failed: %d", env.server_id,
                status);
    }

    return status;
}

dmclt_err_t omadm_controller_interrupt_session()
{
    DM_LOGI("OMADM_C: omadm_controller_interrupt_session");
    is_session_interrupted = true;
    return DMCLT_ERR_NONE;
}

dmclt_err_t omadm_controller_setup_session(omadm_env_t *env)
{
    DM_LOGI("OMADM_C: omadm_controller_setup_session");
    return setup_session(env);
}

/// \todo Temporary solution for restarting DM session
int omadm_controller_restart_session()
{
    DM_LOGI("OMADM_C: omadm_controller_restart_session");
    return (int)restart_session();
}

/// \todo Temporary solution for emulation of SU Cancel request
int omadm_controller_emulate_su_cancel()
{
    DM_LOGI("OMADM_C: omadm_controller_emulate_su_cancel");
    return (int)emulate_su_cancel();
}

int omadm_controller_user_update_session()
{
    DM_LOGI("OMADM_C: omadm_controller_user_update_session");
    return (int)user_update_session();
}
