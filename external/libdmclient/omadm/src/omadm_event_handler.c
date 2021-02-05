/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "omadm_event_handler.h"
#include "dmc_queue.h"
#include "dm_logger.h"

typedef int (*pal_init_t)();
typedef int (*pal_notify_on_omadm_event_t)(process_omadm_event_t);
static pal_init_t pal_init_fn = NULL;
static pal_notify_on_omadm_event_t pal_notify_on_omadm_event_fn = NULL;


result_states_t omadm_event_handler_init()
{
    DM_LOGI("EH: init called");
    int status = RESULT_SUCCESS;
    void * pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    if (NULL != pal_handle) {
        pal_init_fn = dlsym(pal_handle, PAL_INIT);
        if (pal_init_fn != NULL) {
            status = pal_init_fn();
            DM_LOGI("EH: pal_init_fn called | status == %d", (int)status);
            pal_notify_on_omadm_event_fn = dlsym(pal_handle,
                    PAL_NOTIFY_ON_OMADM_EVENT);
            if (pal_notify_on_omadm_event_fn != NULL) {
                status = pal_notify_on_omadm_event_fn(
                        &omadm_event_handler_process_event);
                DM_LOGI("EH: pal_notify_on_omadm_event_fn called |\
                        status == %d", (int)status);
            }
        } else {
			DM_LOGI("EH: pal init fn null");
		}
    } else {
		DM_LOGI("EH: pal handle null");
	}
    DM_LOGI("EH: exit from init | status == %d", (int)status);
    return status;
}

result_states_t omadm_event_handler_process_event(omadm_request_type_t type,
                                                  void *data)
{
    DM_LOGI("EH: call process event type = %d",type);
    dmc_queue_error status = QERR_NONE;
    result_states_t result = RESULT_ERROR;
    unsigned int priority = 0;

    request_t *req = (request_t*) malloc(sizeof(request_t));
    if(!req) {
        DM_LOGE("EH: Cannot allocate memory");
        return RESULT_MEMORY_ERROR;
    }
    memset(req, 0, sizeof(request_t));

    omadm_request_t *dm_req = (omadm_request_t*) malloc(sizeof(omadm_request_t));
    if(!dm_req) {
        free(req);
        req = NULL;
        DM_LOGE("EH: Cannot allocate memory");
        return RESULT_MEMORY_ERROR;
    }
    memset(dm_req, 0, sizeof(omadm_request_t));

    req->handler = OMADM;
    dm_req->package0 = data;
    dm_req->type = type;
    req->request = dm_req;
    priority = dm_req->type == SU_CANCEL ?
                               (DMC_QUEUE_MAX_PRIORITY-1): // highest priority
                                0;   // default priority
    DM_LOGI("EH: event type = %d put to queue with priority %d\n", type, priority);

    status = dmc_events_queue_put(req, priority);
    if (QERR_NONE == status) {
        result = RESULT_SUCCESS;
    } else {
        free(dm_req);
        dm_req = NULL;
        free(req);
        req = NULL;
        result = RESULT_ERROR;
    }
    DM_LOGI("EH: exit from process event | result == %d", (int)result);
    return result;
}

result_states_t omadm_event_handler_process_alert(alert_t *alert)
{
    dmc_queue_error status = QERR_NONE;
    result_states_t result = RESULT_ERROR;

    DM_LOGI("EH: call process alert");
    request_t *req = (request_t*) malloc(sizeof(request_t));
    if(!req) {
        DM_LOGE("EH: Cannot allocate memory");
        return RESULT_MEMORY_ERROR;
    }
    memset(req, 0, sizeof(request_t));

    omadm_request_t *dm_req = (omadm_request_t*) malloc(sizeof(omadm_request_t));
    if(!dm_req) {
        free(req);
        req = NULL;
        DM_LOGE("EH: Cannot allocate memory");
        return RESULT_MEMORY_ERROR;
    }
    memset(dm_req, 0, sizeof(omadm_request_t));

    req->handler = OMADM;
    dm_req->alert = alert;
    dm_req->type = GENERIC_ALERT;
    req->request = dm_req;
    status = dmc_events_queue_put(req, 0);
    if (QERR_NONE == status) {
        result = RESULT_SUCCESS;
    } else {
        free(dm_req);
        dm_req = NULL;
        free(req);
        req = NULL;
        result = RESULT_ERROR;
    }

    DM_LOGI("EH: exit from process alert | result == %d", (int)result);
    return result;
}

