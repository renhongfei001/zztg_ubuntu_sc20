/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef DMC_QUEUE_H_
#define DMC_QUEUE_H_

/**
 * @enum MOCKUP request_handler_t
 * @brief Request handler types
*/
typedef enum {
    OMADM, ///< request for omadm handler
    UDM    ///< request for udm handler
} request_handler_t;

/// @brief MOCKUP Queue request structure
typedef struct {
    request_handler_t handler; ///< type of omadm handler
    void *request;             ///< request structure
} request_t;

/**
 * @enum MOCKUP dmc_queue_error
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
 * @brief MOCKUP dmc_events_queue_put
 *
 * Note: queue doesn't store the item, only pointer to it, ownership of this
 * memory transfered from writer to reader.
 * @param [in] item request to be added to queue
 * @return <code>0</code> if operation succeded, <code>>0</code> otherwise
 */
dmc_queue_error dmc_events_queue_put(request_t *item, unsigned int priority);


#endif /* DMC_QUEUE_H_ */
