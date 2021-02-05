/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * @file controller.c
 * @brief Implementation of main DM client's controller
 */

#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>

#include "controller.h"
#include "dmc_queue.h"
#include "dm_common.h"
#include "dm_logger.h"
#include "omadmclient.h"
#include "net_manager.h"
#include "pal_common.h"

static void *dmc_queue_processor_thread(void *arg);
static pthread_t queue_processor_thread = 0;

extern dmclt_err_t omadm_controller_start_session(void * request);
extern dmclt_err_t omadm_controller_interrupt_session();

bool init_dmcController()
{
    DM_LOGI("DM_Controller initialization started");
    bool res = true;

    if(QERR_NONE == dmc_events_queue_start()) {
        if (pthread_create( &queue_processor_thread, NULL,
                            dmc_queue_processor_thread, NULL)) {
            /* thread was not created. Need stop and release queue. */
            DM_LOGE("Can't start queue processor thread");
            dmc_events_queue_stop();
            dmc_events_queue_release();
            res = false;
        }
    } else {
        DM_LOGE("Can't initialize main queue");
        res = false;
    }
    DM_LOGI("DM_Controller initialized.");
    return res;
}

void stop_dmcController()
{
    DM_LOGI("Stopping DM_Controller");
    dmc_events_queue_stop();
    void *res = 0;
    if (pthread_join(queue_processor_thread, &res)) {
      DM_LOGE("Can't join the queue processor thread.");
    }
    queue_processor_thread = 0;
    dmc_events_queue_release();
    DM_LOGI("DM_Controller stopped");
}

void *dmc_queue_processor_thread(void *arg)
{
    DM_UNUSED(arg);
    DM_LOGI("Started queue processor thread");
    while(1){
        if(dmc_events_queue_empty_lock()) {
            break;
        }
        request_t *req = dmc_events_queue_get();
        if (!req) {
            break;
        }
        DM_LOGI("GOT SOMETHING!!!");
        DM_LOGI("message for %s", req->handler == 0 ? "OMADM" : "UDM");
        if(OMADM == req->handler) {
            DM_LOGI("Consuming request for OMADM controller");
            dmclt_err_t hr_status = omadm_controller_handle_request(req->request);
            if(DMCLT_ERR_NONE != hr_status) {
                DM_LOGE("Handle request failed with error: %d", hr_status);
            }
            // At this point we have finished with OMADM request. Can free it here.
            dmc_queue_release_item(req);
        } else if (UDM == req->handler) {
            /// \todo call appropriate function from UDM controller
            DM_LOGI("Consuming request for UDM controller");
        } else {
            DM_LOGI("Consuming unknown request");
        }

        // Completely free request holder
        free(req);
        req = NULL;
    }
    DM_LOGI("Queue processor thread stopped");
    return 0;
}

void dmc_roaming_enabled()
{
    DM_LOGI("dmc_roaming_enabled");
    // Interrupt OMADM session if roaming is turn on
    omadm_controller_interrupt_session();
}
