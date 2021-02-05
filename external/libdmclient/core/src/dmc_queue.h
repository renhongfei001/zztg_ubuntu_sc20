/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef DMC_QUEUE_H
#define DMC_QUEUE_H

/*!
 * @file dmc_queue.h
 *
 * @brief Queue of incoming events for DMClient
 *
 * Implementation of the input events queue.
 */

#ifdef __cplusplus
extern "C" {
#endif

/// \todo identify proper value for queue size
#ifdef ENABLE_TEST
#define DMC_QUEUE_MAX_SIZE 10
#define DMC_QUEUE_MAX_PRIORITY 2
#else
#define DMC_QUEUE_MAX_SIZE 10
#define DMC_QUEUE_MAX_PRIORITY 2
#endif

/**
 * @enum request_handler_t
 * @brief Request handler types
*/
typedef enum {
    OMADM, ///< request for omadm handler
    UDM    ///< request for udm handler
} request_handler_t;

/// @brief Queue request structure
typedef struct {
    request_handler_t handler; ///< type of omadm handler
    void *request;             ///< request structure
} request_t;

typedef struct _dmc_events_queue dmc_events_queue;

/**
 * @enum dmc_queue_error
 * @brief Error codes of queue related operations.
*/
typedef enum _dmc_queue_error {
    QERR_NONE = 0,              ///< Everything is OK
    QERR_NOT_READY,             ///< Queue not initialized
    QERR_ALREADY_INITIALIZED,   ///< Trying to init already active queue
    QERR_NO_MEMORY,             ///< Required memory couldn't be allocated
    QERR_FULL,                  ///< Queue is full, try again later
    QERR_WRONG_PARAMETERS       ///< One or several parameters are wrong
} dmc_queue_error;

/**
 *@brief Initializes set of queues for each priority
 *
 * @return Status of execution
 * @note  Initialization breaks on error for the whole set. There will be no attempts to
 * initialize other priorities.
 */
dmc_queue_error dmc_events_queue_start(void);

/**
 * @brief Stops set of queues for each priority
 *
 * @return Status of execution
 * @note  Function will try to stop all queues in the set for each priority.
 */
dmc_queue_error dmc_events_queue_stop(void);

/**
 * @brief Puts an event to queue with a given priority
 *
 * @param [in] item request to be added to queue
 * @param [in] priority of the request
 * @return Status of execution
 * @note Execution breaks if queue is not initialized or stopped
 */
dmc_queue_error dmc_events_queue_put(request_t *item, unsigned int priority);

/**
 *@brief Gets an event from queue according to event priority
 *
 * @return Status of execution
 * @note Execution breaks if any queue in the set is not initialized or stopped. No attempts
 * will be made for other priorities.
 */
request_t *dmc_events_queue_get(void);

/**
 * Releases set of queues for each priority
 *
 * @return Status of execution
 * @note  Function will try to release all queues in the set for each priority.
 */
dmc_queue_error dmc_events_queue_release(void);


/**
 * Releases queue item and frees allocated memory
 *
 * @param [in] pointer to request data structure
 * @note Keep in sync with actual data structures !!
 * @see  For data strucures: dm_common.h
 * Also look for usage of dmc_events_queue_put() in
 * omadm_event_handler_process_event(), omadm_event_handler_process_alert()
 */
void dmc_queue_release_item(request_t* request);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // DMC_QUEUE_H
