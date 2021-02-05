/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "dmc_queue.h"
#include "dm_common.h"

#ifndef DMC_QUEUE_MAX_SIZE
#define DMC_QUEUE_MAX_SIZE 10
#endif


int init_suite(void)
{
    dmc_queue_error er = dmc_events_queue_start();
    return (QERR_NONE == er ? 0 : 1);
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite(void)
{
// assuming that stop&release functions work fine when queue not initialized.
    dmc_events_queue_stop();
    dmc_events_queue_release();
    return 0;
}

void test_queue_start_stop(void)
{
    // init&start queue
    dmc_queue_error er = dmc_events_queue_start();
    CU_ASSERT_EQUAL(QERR_NONE, er);
    // Fill up queue
    test_q_put_many();
    // stop queue
    int res = dmc_events_queue_stop();
    CU_ASSERT_EQUAL(0, res);
    // cleanup
    dmc_events_queue_release();
    // just verifying that nothing crashes
}

void test_queue_stop_bad_input(void)
{
    dmc_queue_error er = dmc_events_queue_start();
    CU_ASSERT_EQUAL(QERR_NONE, er);
    // check that queue can't be started twice
    er = dmc_events_queue_start();
    CU_ASSERT_EQUAL(QERR_ALREADY_INITIALIZED, er);
    // stop active queue
    er = dmc_events_queue_stop();
    CU_ASSERT_EQUAL(QERR_NONE, er);
    // try to stop already stopped queue - for now it's acceptable case,
    // i.e. not leading to error. We just doing nothing
    er = dmc_events_queue_stop();
    CU_ASSERT_EQUAL(QERR_NONE, er);
    dmc_events_queue_release();
    // just shouldn't crash, at least for now
}

void test_queue_release_bad(void)
{
    dmc_queue_error er = dmc_events_queue_release();
    CU_ASSERT_EQUAL(QERR_NOT_READY, er);
    er = dmc_events_queue_start();
    CU_ASSERT_EQUAL(QERR_NONE, er);
    er = dmc_events_queue_release();
    CU_ASSERT_EQUAL(QERR_NOT_READY, er);
    // stop active queue
    er = dmc_events_queue_stop();
    CU_ASSERT_EQUAL(QERR_NONE, er);
    er = dmc_events_queue_release();
    CU_ASSERT_EQUAL(QERR_NONE, er);
}


void test_queue_put_single_0(void)
{
    request_t *tst = malloc(sizeof(request_t));
    CU_ASSERT_PTR_NOT_NULL(tst);
    tst->handler = OMADM;
        omadm_request_t* omadm_rq = (omadm_request_t*)malloc(sizeof(omadm_request_t));
        memset(omadm_rq, 0, sizeof(omadm_request_t));
        omadm_rq->package0 = (buffer_t*)malloc(sizeof(buffer_t));
        memset(omadm_rq->package0, 0, sizeof(buffer_t));
        omadm_rq->package0->buffer = (uint8_t*)malloc(50);
        memset(omadm_rq->package0->buffer, 0xff, 50);
        omadm_rq->package0->len = 50;
        tst->request = omadm_rq;

    dmc_queue_error er = dmc_events_queue_put(tst, 0);
    CU_ASSERT_EQUAL(QERR_NONE, er);
}

void test_queue_put_single_1(void)
{
    request_t *tst = malloc(sizeof(request_t));
    CU_ASSERT_PTR_NOT_NULL(tst);
    tst->handler = OMADM;
        omadm_request_t* omadm_rq = (omadm_request_t*)malloc(sizeof(omadm_request_t));
        memset(omadm_rq, 0, sizeof(omadm_request_t));

        omadm_rq->alert = (alert_t*)malloc(sizeof(alert_t));
        memset(omadm_rq->alert, 0, sizeof(alert_t));

        omadm_rq->alert->alert_data = (char*)malloc(50);
        memset(omadm_rq->alert->alert_data, 0xff, 50);

        omadm_rq->alert->mo = (char*)malloc(50);
        memset(omadm_rq->alert->mo, 0xff, 50);

        tst->request = omadm_rq;
    dmc_queue_error er = dmc_events_queue_put(tst, 1);
    CU_ASSERT_EQUAL(QERR_NONE, er);
}


void test_queue_put_single_bad(void)
{
    request_t *tst = malloc(sizeof(request_t));
    CU_ASSERT_PTR_NOT_NULL(tst);
    tst->handler = OMADM;
    tst->request = NULL;
    dmc_queue_error er = dmc_events_queue_put(tst, 5);
    CU_ASSERT_EQUAL(QERR_WRONG_PARAMETERS, er);
}


void test_q_put_many(void)
{
    request_t *tst = NULL;
    int i;
    for(i=0; i < (DMC_QUEUE_MAX_SIZE + 2); i++) {
        tst = malloc(sizeof(request_t));
        CU_ASSERT_PTR_NOT_NULL(tst);
        // set first event as OMADM type
        tst->handler = i ? UDM : OMADM;
        omadm_request_t* omadm_rq = (omadm_request_t*)malloc(sizeof(omadm_request_t));
        memset(omadm_rq, 0, sizeof(omadm_request_t));
        if(tst->handler == OMADM) {
            omadm_rq->alert = (alert_t*)malloc(sizeof(alert_t));
            memset(omadm_rq->alert, 0, sizeof(alert_t));
            omadm_rq->alert->alert_data = (char*)malloc(50);
            memset(omadm_rq->alert->alert_data, 0xff, 50);
            omadm_rq->alert->mo = (char*)malloc(50);
            memset(omadm_rq->alert->mo, 0xff, 50);
        }
        tst->request = omadm_rq;
        dmc_queue_error result = dmc_events_queue_put(tst, 0);
        if (i < DMC_QUEUE_MAX_SIZE) {
            CU_ASSERT_EQUAL(QERR_NONE, result);
        } else {
            CU_ASSERT_EQUAL(QERR_FULL, result);
        }
    }
    for(i=0; i < (DMC_QUEUE_MAX_SIZE + 2); i++) {
        tst = malloc(sizeof(request_t));
        CU_ASSERT_PTR_NOT_NULL(tst);
        // set first event as OMADM type
        tst->handler = i ? UDM : OMADM;
        omadm_request_t* omadm_rq = (omadm_request_t*)malloc(sizeof(omadm_request_t));
        memset(omadm_rq, 0, sizeof(omadm_request_t));
        if(tst->handler == OMADM) {
            omadm_rq->package0 = (buffer_t*)malloc(sizeof(buffer_t));
            memset(omadm_rq->package0, 0, sizeof(buffer_t));
            omadm_rq->package0->buffer = (uint8_t*)malloc(50);
            memset(omadm_rq->package0->buffer, 0xff, 50);
            omadm_rq->package0->len = 50;
        }
        tst->request = omadm_rq;
        dmc_queue_error result = dmc_events_queue_put(tst, 1);
        if (i < DMC_QUEUE_MAX_SIZE) {
            CU_ASSERT_EQUAL(QERR_NONE, result);
        } else {
            CU_ASSERT_EQUAL(QERR_FULL, result);
        }
    }
}

void test_q_get_many(void)
{
    //expecting first event "OMADM" type
    request_t *tst = NULL;
    tst = dmc_events_queue_get();
    CU_ASSERT_PTR_NOT_NULL(tst);
    CU_ASSERT_EQUAL(tst->handler, OMADM);
    dmc_queue_release_item(tst);
    free(tst);
    tst = NULL;

    int i;
    for(i=0; i < (DMC_QUEUE_MAX_SIZE-1); i++) {
        tst = dmc_events_queue_get();
        CU_ASSERT_PTR_NOT_NULL(tst);
        // expecting other events in the queue "UDM" type
        CU_ASSERT_EQUAL(tst->handler, UDM);
        dmc_queue_release_item(tst);
        free(tst);
        tst = NULL;
    }

    //expecting first event "OMADM" type
    tst = dmc_events_queue_get();
    CU_ASSERT_PTR_NOT_NULL(tst);
    CU_ASSERT_EQUAL(tst->handler, OMADM);
    dmc_queue_release_item(tst);
    free(tst);
    tst = NULL;

    for(i=0; i < (DMC_QUEUE_MAX_SIZE-5); i++) {
        tst = dmc_events_queue_get();
        CU_ASSERT_PTR_NOT_NULL(tst);
        // expecting other events in the queue "UDM" type
        CU_ASSERT_EQUAL(tst->handler, UDM);
        dmc_queue_release_item(tst);
        free(tst);
        tst = NULL;
    }
}


void test_queue_get_single(void)
{
    // WARNING: this test expected to be run after single 'put' to queue
    request_t *tst = dmc_events_queue_get();
    CU_ASSERT_PTR_NOT_NULL(tst);
    CU_ASSERT_EQUAL(tst->handler, OMADM);
    dmc_queue_release_item(tst);
    free(tst);
}

/// \todo implement multithreaded testing
void test_q_read_from_empty(void)
{
    // try to read one more time from empty queue (possible endless loop NOW)
    // correctly it shall be tested using 2 threads (reader & writer)
    // bad idea ;)
//    tst = dmc_events_queue_get();
//    CU_ASSERT_PTR_NULL(tst);
}

void test_queue_get_two(void)
{
    // WARNING: this test expected to be run right after test_q_put_many()
    request_t *tst = dmc_events_queue_get();
    CU_ASSERT_PTR_NOT_NULL(tst);
    CU_ASSERT_EQUAL(tst->handler, UDM);
    dmc_queue_release_item(tst);
    free(tst);
    tst = NULL;
    tst = dmc_events_queue_get();
    CU_ASSERT_PTR_NOT_NULL(tst);
    CU_ASSERT_EQUAL(tst->handler, UDM);
    dmc_queue_release_item(tst);
    free(tst);
    tst = NULL;
}

void test_queue_get_put_dead_queue(void)
{
    // don't forget to re-start queue to allow correct cleanup (maybe)
    dmc_queue_error er = dmc_events_queue_stop();
    CU_ASSERT_EQUAL(QERR_NONE, er);
    request_t *tst = malloc(sizeof(request_t));
    CU_ASSERT_PTR_NOT_NULL(tst);
    tst->handler = OMADM;
    tst->request = NULL;
    er = dmc_events_queue_put(tst, 0);
    CU_ASSERT_EQUAL(QERR_NOT_READY, er);
    free(tst);
    tst = NULL;
    request_t *req = dmc_events_queue_get();
    CU_ASSERT_PTR_NULL(req);
    er = dmc_events_queue_release();
    CU_ASSERT_EQUAL(QERR_NONE, er)
    tst = malloc(sizeof(request_t));
    CU_ASSERT_PTR_NOT_NULL(tst);
    tst->handler = OMADM;
    tst->request = NULL;
    er = dmc_events_queue_put(tst, 1);
    CU_ASSERT_EQUAL(QERR_NOT_READY, er);
    free(tst);
    tst = NULL;
    req = dmc_events_queue_get();
    CU_ASSERT_PTR_NULL(req);
}


int main(){
    // start&stop queue tests
    CU_TestInfo start_stop_tests[] = {
        {"Start events queue good case", test_queue_start_stop},
        {"Start started & stop stopped", test_queue_stop_bad_input},
        {"Release not inited and not stopped", test_queue_release_bad},
        CU_TEST_INFO_NULL,
    };

    // queue operations tests
    CU_TestInfo queue_ops_tests[] = {
        {"Add new event to queue with priority 0", test_queue_put_single_0},
        {"Get event from queue", test_queue_get_single},
        {"Add new event to queue with priority 1", test_queue_put_single_1},
        {"Get event from queue", test_queue_get_single},
        {"Add new event with bad priority", test_queue_put_single_bad},
        {"Add events up to queue overflow", test_q_put_many},
        {"Get some events from queue", test_q_get_many},
        {"Get 2 items from queue", test_queue_get_two},
        {"Get/Put dead queue", test_queue_get_put_dead_queue},
        CU_TEST_INFO_NULL,
    };

    CU_SuiteInfo suites[] = {
        {"DMC events queue start/stop suite", NULL, NULL, start_stop_tests},
        {"DMC events queue operations suite", init_suite, clean_suite, queue_ops_tests},
        CU_SUITE_INFO_NULL,
    };

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    if( CU_register_suites( suites ) != CUE_SUCCESS ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int failedTests = CU_get_number_of_tests_failed();
    if(failedTests > 0)
    {
        printf("Failure list:");
        CU_basic_show_failures(CU_get_failure_list());
        printf("\n");
    }
    CU_ErrorCode errorResult = CU_get_error();
    CU_cleanup_registry();
    return errorResult == CUE_SUCCESS ? failedTests : errorResult;
}

