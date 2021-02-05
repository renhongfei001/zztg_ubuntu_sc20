/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

//#include "controller.h"
#include "dmc_queue.h"
#include "dm_logger.h"
#include "net_manager.h"
#include "dm_common.h"

extern bool init_dmcController(void);
extern void stop_dmcController(void);

int init_suite(void)
{
  return 0;
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite(void)
{
    return 0;
}

void test_init_controller(void)
{
    DM_LOGI("**** test_init_controller {");
    CU_ASSERT(netm_init_network_manager());
    //check that init works somehow
    CU_ASSERT(init_dmcController());
    sleep(1);
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
    request_t *tst2 = malloc(sizeof(request_t));
    CU_ASSERT_PTR_NOT_NULL(tst);
    tst2->handler = UDM;
    tst2->request = NULL;
    er = dmc_events_queue_put(tst2, 0);
    CU_ASSERT_EQUAL(QERR_NONE, er);
    sleep(1);
    // just check that nothing crashed
    stop_dmcController();
    netm_stop_network_manager();
    DM_LOGI("**** test_init_controller }");
}

void test_controller_network_unavailable(void)
{
    DM_LOGI("**** test_controller_network_unavailable {");
    CU_ASSERT(netm_init_network_manager());
    network_conditions_t event;
    event.net_feature = NETWORK_WIFI_CONNECTED;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    CU_ASSERT(init_dmcController());
    sleep(1);
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
    request_t *tst2 = malloc(sizeof(request_t));
    CU_ASSERT_PTR_NOT_NULL(tst);
    tst2->handler = UDM;
    tst2->request = NULL;
    er = dmc_events_queue_put(tst2, 0);
    CU_ASSERT_EQUAL(QERR_NONE, er);
    event.net_feature = NETWORK_WIFI_CONNECTED;
    event.enabled = true;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    stop_dmcController();
    netm_stop_network_manager();
    DM_LOGI("**** test_controller_network_unavailable }");
}

void test_controller_celular_alone(void)
{
    DM_LOGI("**** test_controller_celular_alone {");
    CU_ASSERT(netm_init_network_manager());
    network_conditions_t event;
    event.net_feature = NETWORK_CELLULAR_CONNECTED;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    CU_ASSERT(init_dmcController());
    sleep(1);
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
    request_t *tst2 = malloc(sizeof(request_t));
    CU_ASSERT_PTR_NOT_NULL(tst);
    tst2->handler = UDM;
    tst2->request = NULL;
    er = dmc_events_queue_put(tst2, 0);
    CU_ASSERT_EQUAL(QERR_NONE, er);
    event.net_feature = NETWORK_CELLULAR_CONNECTED;
    event.enabled = true;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    stop_dmcController();
    netm_stop_network_manager();
    DM_LOGI("**** test_controller_celular_alone }");
}


void test_controller_roaming_enabled(void)
{
     DM_LOGI("**** test_controller_roaming_enabled {");
     CU_ASSERT(netm_init_network_manager());
     network_conditions_t event;
     event.net_feature = NETWORK_DATA_ROAMING;
     event.enabled = true;
     CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
     CU_ASSERT(init_dmcController());
     sleep(2);
     stop_dmcController();
     netm_stop_network_manager();
     DM_LOGI("**** test_controller_rouming_enabled }");
}

void test_init_destroy_empty(void)
{
    DM_LOGI("**** test_init_destroy_empty {");
    //check that init works somehow
    CU_ASSERT(init_dmcController());
    sleep(1);
    // check that nothing crashed while destroying a mutex during cond_wait
    stop_dmcController();
    DM_LOGI("**** test_init_destroy_empty }");
}

int main(){
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("DM controller suite", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if (NULL == CU_add_test(pSuite, "test init main controller", test_init_controller))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (NULL == CU_add_test(pSuite, \
                           "test init&destory controller with empty \queue", \
                           test_init_destroy_empty))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }
   if (NULL == CU_add_test(pSuite,
                           "test controller network unavailable",
                           test_controller_network_unavailable))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }
   if (NULL == CU_add_test(pSuite,
                           "test controller celular alone",
                           test_controller_celular_alone))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }
   if (NULL == CU_add_test(pSuite,
                           "test controller roaming enabled",
                           test_controller_roaming_enabled))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_SILENT);
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
