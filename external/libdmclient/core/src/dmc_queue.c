/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdbool.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>

#include "dmc_queue.h"

/// \todo add macro check for 'common::utils' availability
#include "dm_common.h"
#include "dm_logger.h"

/** @struct _dmc_queue_item
 * @brief Single item of main DMC queue
 */
typedef struct _dmc_queue_item {
    struct _dmc_queue_item *next;   ///< link to next queue item
    request_t *data;                ///< content of current item
} dmc_queue_item;

/** @struct _dmc_events_queue
 * @brief Main DMC queue
 */
struct _dmc_events_queue {
    dmc_queue_item *list;           ///< first item in the queue
    int is_alive;                   ///< shows if the queue is able to receive/provide items
    pthread_mutex_t lock;           ///< just for multi threaded syncronization
    pthread_cond_t can_read;        ///< way to notify <code>reader</code> that queue not empty
    unsigned int priority;          ///< priority: 0 - lowest, (DMC_QUEUE_MAX_PRIORITY-1) - highest
};

static dmc_events_queue* queue_holder[DMC_QUEUE_MAX_PRIORITY];

static dmc_queue_item *dmc_queue_create_item(request_t *data);


dmc_queue_error dmc_events_queue_start(void)
{
    int i;
    for(i=0; i<DMC_QUEUE_MAX_PRIORITY; i++) {
        DM_LOGI("dmc_events_queue_start() priority %d", i);

        if (NULL != queue_holder[i]) {
            DM_LOGE("Queue(%d) already initialized", i);
            return QERR_ALREADY_INITIALIZED;
        }

        queue_holder[i] = malloc(sizeof(dmc_events_queue));
        if(!queue_holder[i]) {
            DM_LOGE("Failed to allocate memory for queue(%d)", i);
            return QERR_NO_MEMORY;
        }
        queue_holder[i]->is_alive = 0;
        queue_holder[i]->list = NULL;

        // initialize common mutex only for main queue
        if (0 == i) {
            if (pthread_mutex_init(&queue_holder[0]->lock, NULL)) {
                DM_LOGE("Failed to init mutex for main queue");
                free(queue_holder[0]);
                return QERR_NOT_READY;
            }
            if (pthread_cond_init(&queue_holder[0]->can_read, NULL)) {
                DM_LOGE("Failed to init conditional for main queue");
                pthread_mutex_destroy(&queue_holder[0]->lock);
                free(queue_holder[0]);
                return QERR_NOT_READY;
            }
        }

        queue_holder[i]->is_alive = 1;
        queue_holder[i]->priority = (unsigned int)i;
    }
    DM_LOGI("DM queues initialized");
    return QERR_NONE;
}

dmc_queue_error dmc_events_queue_stop(void)
{
    dmc_queue_error err = QERR_NONE;

    if (!queue_holder[0]) {
        DM_LOGE("Main queue is not initialised");
        return QERR_NOT_READY;
    }
    pthread_mutex_lock(&queue_holder[0]->lock);
    int i;
    for(i=0; i<DMC_QUEUE_MAX_PRIORITY; i++) {
        DM_LOGI("dmc_events_queue_stop() priority %d", i);

        if (!queue_holder[i]) {
            DM_LOGE("Queue(%d) is not initialized", i);
            err = QERR_NOT_READY;
            continue;
        }

        if (0 == queue_holder[i]->is_alive) {
            DM_LOGD("Queue(%d) is already stopped. Nothing to do", i);
            continue;
        }
        queue_holder[i]->is_alive = 0;
    }
    // unblock reader to allow finish waiting
    pthread_cond_broadcast(&queue_holder[0]->can_read);
    pthread_mutex_unlock(&queue_holder[0]->lock);

    if(QERR_NONE == err)
        DM_LOGI("DM queues stopped");
    return err;
}

bool dmc_events_queue_empty_lock(void)
{
    if (!queue_holder[0]) {
        DM_LOGE("Main queue is not initialised");
        return true;
    }
    pthread_mutex_lock(&queue_holder[0]->lock);
    int i;
    for(i=(DMC_QUEUE_MAX_PRIORITY-1); i>=0; i--) {
        DM_LOGI("dmc_events_queue_empty_lock() try priority %d", i);

        if (!queue_holder[i]) {
            DM_LOGE("Queue(%d) is not initialised", i);
            pthread_mutex_unlock(&queue_holder[0]->lock);
            return true;
        }
        if(queue_holder[i]->priority > 0 &&
                !queue_holder[i]->list && queue_holder[i]->is_alive) {
            DM_LOGI("Queue(%d) is empty. Check next priority", i);
            continue;
        }

        // We are getting here only for lowest priority - 0
        if(!queue_holder[i]->list && queue_holder[i]->is_alive) {
            DM_LOGD("Queue(%d) is empty. Waiting...", i);
            int wait_res = pthread_cond_wait(&queue_holder[0]->can_read, &queue_holder[0]->lock);
            DM_LOGD("Wait ended. res = %d", wait_res);
        }
        break;
    }
    pthread_mutex_unlock(&queue_holder[0]->lock);
    return false;
}

dmc_queue_error dmc_events_queue_put(request_t *item, unsigned int priority)
{
    if(priority >= DMC_QUEUE_MAX_PRIORITY) {
        DM_LOGE("Wrong priority. Priority=%d Max=%d", priority,
            (DMC_QUEUE_MAX_PRIORITY -1));
        return QERR_WRONG_PARAMETERS;
    }

    DM_LOGI("dmc_events_queue_put_item(): handler %s priority %d", item ? \
                (item->handler == OMADM ? "OMADM" : "UDM" ) : "item is NULL", priority);

    if (!queue_holder[0] || !queue_holder[priority]) {
        DM_LOGE("Main queue or queue(%d) not initialised", priority);
        return QERR_NOT_READY;
    }
    pthread_mutex_lock(&queue_holder[0]->lock);
    if(!queue_holder[priority]->is_alive) {
        DM_LOGE("Queue(%d) is stopped", priority);
        pthread_mutex_unlock(&queue_holder[0]->lock);
        return QERR_NOT_READY;
    }
    dmc_queue_error err = QERR_NONE;
    dmc_queue_item *list = queue_holder[priority]->list;
    // queue is empty
    if (!list) {
        queue_holder[priority]->list = dmc_queue_create_item(item);
    } else {
        // at least 1 item in queue + new one
        int count = 2;
        while (list->next) {
            count++;    // just walk through and make some math
            list = list->next;
        }
        if (count <= DMC_QUEUE_MAX_SIZE) {
            list->next = dmc_queue_create_item(item);
        } else {
            DM_LOGI("Reached max queue(%d) length", priority);
            err = QERR_FULL;
        }
    }
    pthread_cond_broadcast(&queue_holder[0]->can_read);
    pthread_mutex_unlock(&queue_holder[0]->lock);
    return err;
}

request_t *dmc_events_queue_get(void)
{
    request_t* req = NULL;
    if (!queue_holder[0]) {
        DM_LOGE("Main queue is not initialised");
        return req;
    }

    pthread_mutex_lock(&queue_holder[0]->lock);
    int i;
    for(i=(DMC_QUEUE_MAX_PRIORITY-1); i>=0; i--) {
        DM_LOGI("dmc_events_queue_get_item() try priority %d", i);

        if (!queue_holder[i]) {
            DM_LOGE("Queue(%d) is not initialised", i);
            pthread_mutex_unlock(&queue_holder[0]->lock);
            return req;
        }
        if(queue_holder[i]->priority > 0 &&
                !queue_holder[i]->list && queue_holder[i]->is_alive) {
            DM_LOGI("Queue(%d) is empty. Check next priority", i);
            continue;
        }

        // We are getting here only for lowest priority - 0
        if(!queue_holder[i]->list && queue_holder[i]->is_alive) {
            DM_LOGE("Queue(%d) is empty.", i);
            continue;
        }
        dmc_queue_item *item = queue_holder[i]->list;

        if (item) {
          queue_holder[i]->list = item->next;
        }

        req = item ? item->data : NULL;
        free(item);
        DM_LOGD("got message for: %d.", req ? req->handler : 9);
        break;
    }
    pthread_mutex_unlock(&queue_holder[0]->lock);
    return req;
}

dmc_queue_item *dmc_queue_create_item(request_t *data)
{
    dmc_queue_item *item = malloc(sizeof(dmc_queue_item));
    if(item) {
        item->next = NULL;
        item->data = data;
        DM_LOGD("Created item is for = %d", item->data ? item->data->handler : 9);
    }

    return item;
}

void dmc_queue_release_item(request_t* request)
{
    if(request) {
        if(request->request) {
            if(OMADM == request->handler) {
                omadm_request_t* omadm_rq = request->request;
                if(omadm_rq->package0) {
                    if((omadm_rq->package0->len)&&(omadm_rq->package0->buffer)) {
                        free(omadm_rq->package0->buffer);
                        omadm_rq->package0->buffer = NULL;
                        omadm_rq->package0->len = 0;
                    }
                free(omadm_rq->package0);
                omadm_rq->package0 = NULL;
                }
                if(omadm_rq->alert) {
                    if(omadm_rq->alert->alert_data) {
                        free(omadm_rq->alert->alert_data);
                        omadm_rq->alert->alert_data = NULL;
                    }
                    if(omadm_rq->alert->mo) {
                        free(omadm_rq->alert->mo);
                        omadm_rq->alert->mo = NULL;
                    }
                free(omadm_rq->alert);
                omadm_rq->alert = NULL;
                }
            }else if(UDM == request->handler) {
                // Add correspondind actions here
            }
        free(request->request);
        request->request = NULL;
        }
    }
}

dmc_queue_error dmc_events_queue_release(void)
{
    dmc_queue_error err = QERR_NONE;
    if (!queue_holder[0] || queue_holder[0]->is_alive) {
        DM_LOGE("Main queue is not initialised or not stopped");
        return QERR_NOT_READY;
    }
    pthread_mutex_lock(&queue_holder[0]->lock);
    unsigned int i;
    for(i=0; i<DMC_QUEUE_MAX_PRIORITY; i++) {
        DM_LOGI("dmc_events_queue_release() priority %d", i);

        if (!queue_holder[i]) {
            DM_LOGE("Queue(%d) is not initialized", i);
            err = QERR_NOT_READY;
            continue;
        }
        if (queue_holder[i]->is_alive) {
            DM_LOGE("Queue(%d) is not stopped", i);
            err = QERR_NOT_READY;
            continue;
        }
        // cleanup queue, if anything left there
        if (queue_holder[i]->list) {
            dmc_queue_item *list = queue_holder[i]->list;
            while(list->next) {
                dmc_queue_item *tmp = list;
                list = list->next;
                dmc_queue_release_item(tmp->data);
                free(tmp->data);
                tmp->data = NULL;
                free(tmp);
                tmp = NULL;
            }
        free(list);
        list = NULL;
        }
        // don't free main queue handler yet
        if(i != 0) {
            free(queue_holder[i]);
            queue_holder[i] = NULL;
        }
    }
    // release main queue
    pthread_mutex_unlock(&queue_holder[0]->lock);
    pthread_mutex_destroy(&queue_holder[0]->lock);
    pthread_cond_destroy(&queue_holder[0]->can_read);
    free(queue_holder[0]);
    queue_holder[0] = NULL;


    if(QERR_NONE == err)
        DM_LOGI("DM queues released");
    return err;
}
