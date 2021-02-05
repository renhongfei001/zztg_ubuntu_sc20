/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "controller.h"
#include "dm_logger.h"
#include "net_manager.h"

/**
 * @file net_manager.c
 * @brief Implementation of cellular network manager
 */

static network_conditions_state_t state;

// function for register callback
typedef int (*pal_notify_on_conditions_change_t)(network_conditions_changed_t);
static pal_notify_on_conditions_change_t pal_notify_fn = NULL;
// function for unregister callback
typedef int(*pal_unregister_callback_t)(int);
static pal_unregister_callback_t pal_unregister_callback_fn = NULL;

static int idCallback = -1;
static void * pal_handle = NULL;

pthread_mutex_t lock;    ///< just for multi threaded syncronization
pthread_cond_t net_changed; ///< way to notify that network is ready for sending

typedef int (*pal_request_admin_network_t)(bool);
static pal_request_admin_network_t pal_request_admin_network_fn = NULL;

static int admin_apn_status = ADMIN_NET_UNKNOWN;

void netm_up_admin_network()
{
	int try_time = 0;
    if (ADMIN_NET_AVAILABLE == admin_apn_status) {
        DM_LOGD("NM: admin apn already established");
        return;
    }
    pal_request_admin_network_fn = dlsym(pal_handle,
               PAL_REQUEST_ADMIN_APN);
    if (!netm_is_wifi_connected()) {
        DM_LOGD("NM: Try to switch ON admin apn");
        pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
        if(pal_handle == NULL) {
            DM_LOGE("NM: can't open PAL lib %s", dlerror());
            // dmclient shall continue work
        } else {
            if(pal_request_admin_network_fn == NULL) {
                DM_LOGE("NM: pal_request_admin_network not found!");
                // dmclient shall continue work
            } else {
                admin_apn_status = pal_request_admin_network_fn(true);
                DM_LOGD("NM: SSSwitch ON admin apn - status is [%d]", admin_apn_status);
            }
       }
			
    } else {
        DM_LOGD("NM: Is not cellular data connection. Continue without switch of admin apn.");
    }
}

void netm_down_admin_network()
{
   int (*testFunc)() = dlsym(pal_handle, "get_network_status");
   int net_status=0;

    if(testFunc == NULL) {
        DM_LOGE("NM: testFunc not found!");
        // dmclient shall continue work
    } else {
        net_status= testFunc();
        DM_LOGD("NM: net_status status is [%d]", net_status);
    }
 
    if (!netm_is_wifi_connected()) {
        //if (ADMIN_NET_AVAILABLE == admin_apn_status) {
		if (net_status == 1) {
            admin_apn_status = pal_request_admin_network_fn(false);
            DM_LOGE("NM: Switch OFF admin apn - status is [%d]", admin_apn_status);
        }
	} else
	{
       if(net_status==1){
            admin_apn_status = pal_request_admin_network_fn(false);
            DM_LOGE("NM: Switch OFF admin apn - status is [%d]", admin_apn_status);
        }
	}
}

/*
 * Base scenario when DM session should not started
 *
 * WIFI_CONNECTED     | 0 | 0 | 0 | 0 | 0 | 0 |
 * CELLULAR_CONNECTED | 0 | 0 | 1 | 1 | 1 | 1 |
 * ACTIVE_VOICE_CALL  | 0 | 1 | 1 | 1 | 1 | 0 |
 * DATA_ROAMING       | 0 | 0 | 0 | 0 | 1 | 1 |
 * session start      | no| no| no| no| no| no|
*/
bool netm_is_network_ready()
{
    bool ready = false;
    if (state.isWiFiConnected) {
        ready = true;
    } else {
        if (state.isCellularConnected && !state.isActiveVoiceCall) {
            if (state.isDataRoaming) {
                ready = false;
            } else {
                ready = true;
            }
        }
    }
    DM_LOGI("NM: is_network_ready - %d",(int)ready);
    return ready;
}

bool netm_is_wifi_connected()
{
    DM_LOGD("11NM: isCellularConnected [%d], isWiFiConnected [%d]",
            (int)state.isCellularConnected, (int)state.isWiFiConnected);
	return state.isWiFiConnected;
}

bool netm_is_network_ready_lock()
{
    pthread_mutex_lock(&lock);
    while (!netm_is_network_ready()) {
        DM_LOGD("NM: Network is not ready. Waiting...");
        int wait_res = pthread_cond_wait(&net_changed, &lock);
        DM_LOGD("NM: Wait ended. res = %d", wait_res);
    }
    pthread_mutex_unlock(&lock);
    return true;
}

bool netm_init_network_manager()
{
    DM_LOGI("NM: init_net_manager {");
    // initialize mutex and conditional
    if (pthread_mutex_init(&lock, NULL)) {
        DM_LOGE("Failed to init mutex for network manager");
        return false;
    }
    if (pthread_cond_init(&net_changed, NULL)) {
        DM_LOGE("Failed to init conditional for network manager");
        pthread_mutex_destroy(&lock);
        return false;
    }

    state.isActiveVoiceCall = false;
    state.isCellularConnected = false;
    state.isDataRoaming = false;
    state.isWiFiConnected = true;  // for testing

    // Register callback to PAL
    pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    if(pal_handle == NULL) {
        DM_LOGE("NM: can't open PAL lib %s", dlerror());
        return false;
    }
    pal_notify_fn = dlsym(pal_handle,
            PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES);
    if(pal_notify_fn == NULL) {
        DM_LOGE("NM: callback not found!");
        // dmclient shall continue work
        return true;
    }
    idCallback = pal_notify_fn( &netm_handler_process_event);
    pal_unregister_callback_fn = dlsym(pal_handle,
                                    PAL_NOTIFY_STOP_NETWORK_CONDITIONS_CHANGES);
    if(pal_unregister_callback_fn == NULL) {
        DM_LOGI("NM: unregister callback function not found in PAL");
    }
    DM_LOGI("NM: pal_notify_on_net_manager_event_fn called with id == %d", (int)idCallback);

    return true;
}

void netm_stop_network_manager()
{
    DM_LOGI("NM: net manager stopping");
    // unregister callback
    if(pal_unregister_callback_fn) {
        /// todo\ unregister all subscribers
        int res = pal_unregister_callback_fn(0/*idCallback*/);
        DM_LOGI("NM: unregister callback status [%d]",res);
    }
    if( pal_handle )
        dlclose(pal_handle);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&net_changed);
    DM_LOGI("NM: net manager stopped");
}

int netm_handler_process_event(network_conditions_t event)
{
    int status = 0;
    pthread_mutex_lock(&lock);
    DM_LOGI("NM: net_manager_process_event type = %d state = %d",
                                               event.net_feature,
                                             (int)event.enabled);
    switch(event.net_feature) {
    case NETWORK_WIFI_CONNECTED:
        DM_LOGD("NNNM: netm_handler_process_event() NETWORK_WIFI_CONNECTED: [%d]",
                (int)event.enabled);
        state.isWiFiConnected = event.enabled;
        break;
    case NETWORK_ACTIVE_VOICE_CALL:
        DM_LOGD("NM: netm_handler_process_event() NETWORK_ACTIVE_VOICE_CALL: [%d]",
                (int)event.enabled);
        state.isActiveVoiceCall = event.enabled;
        break;
    case NETWORK_CELLULAR_CONNECTED:
        DM_LOGD("NM: netm_handler_process_event() NETWORK_CELLULAR_CONNECTED: [%d]",
                (int)event.enabled);
        state.isCellularConnected = event.enabled;
        break;
    case NETWORK_DATA_ROAMING:
        DM_LOGD("NM: netm_handler_process_event() NETWORK_DATA_ROAMING: [%d]",
                (int)event.enabled);
        state.isDataRoaming = event.enabled;
        // need to interrupt OMADM session if roaming is turn on
        if (state.isDataRoaming) {
            dmc_roaming_enabled();
        }
        break;
    }
    netm_is_network_ready(); //debug only, remove later
    pthread_cond_broadcast(&net_changed);
    pthread_mutex_unlock(&lock);
    return status;
}
