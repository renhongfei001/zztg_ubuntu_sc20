/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef __OMADM_EVENT_HANDLER__
#define __OMADM_EVENT_HANDLER__

#include "internals.h"
#include "omadm_controller.h"
#include "pal.h"

/**
 * Event Handler Initialization
 * Link to PAL and setup event callback through call API function
 * pal_notify_on_omadm_event
 */
result_states_t omadm_event_handler_init();

/**
 * Process WAP push or "Check for update" requests, that
 * require a new OMADM session and puts them to the queue.
 *
 * @param[in] data OMADM Event request data
 * @param[in] type  OMADM request types
 * @return Result code
 */
result_states_t omadm_event_handler_process_event(omadm_request_type_t type,
                                                  void *data);

/**
 * Process alert request, that require a new OMADM session
 * and puts them to the queue.
 *
 * @param[in] alert Alert request data
 * @return Result code
 */
result_states_t omadm_event_handler_process_alert(alert_t *alert);

#endif
