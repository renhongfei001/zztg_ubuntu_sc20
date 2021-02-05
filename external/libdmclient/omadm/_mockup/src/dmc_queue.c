/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include "dmc_queue.h"
#include "dm_logger.h"
#include "omadm_controller.h"


dmc_queue_error dmc_events_queue_put(request_t *item, unsigned int priority) {
    int type = 0;
    if(item && item->request){
        type = ((omadm_request_t *)item->request)->type;
        DM_LOGI("OMADM_C: MOCKUP dmc_events_queue_put type = %d; priority = %d\n",type, priority);
        omadm_controller_handle_request(item->request);
    }
    return QERR_NONE;
}
