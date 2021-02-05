/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdbool.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "dm_logger.h"
#include "net_manager.h"

typedef int (*pal_notify_on_conditions_change_t)(network_conditions_changed_t);
static pal_notify_on_conditions_change_t pal_notify_fn = NULL;


bool netm_is_network_ready();

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

void test_init_net_manager(void)
{
    DM_LOGI("test_init_net_manager");
    CU_ASSERT_TRUE (netm_init_network_manager());
}

void test_stop_net_manager(void)
{
    DM_LOGI("test_init_net_manager");
    netm_stop_network_manager();
}

/*
 * Test emulates notification callback
 */
void test_net_manager_process_event(void)
{
    DM_LOGI("test_net_manager_process_event");
    void * pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    CU_ASSERT_NOT_EQUAL(pal_handle, NULL);

    if (pal_handle != NULL) {
        pal_notify_fn = dlsym(pal_handle,
                                          PAL_NOTIFY_NETWORK_CONDITIONS_CHANGES);
    }
    CU_ASSERT_NOT_EQUAL(pal_notify_fn, NULL);
    if(pal_notify_fn)
        CU_ASSERT_EQUAL(pal_notify_fn(netm_handler_process_event),0);
}

/*
 * Test emulates changes of a network state
 */
void test_net_manager_set_states(void)
{
   network_conditions_t event;
   event.net_feature = NETWORK_WIFI_CONNECTED;
   event.enabled = true;
   CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
   event.net_feature = NETWORK_ACTIVE_VOICE_CALL;
   event.enabled = false;
   CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
   event.net_feature = NETWORK_CELLULAR_CONNECTED;
   event.enabled = false;
   CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
   CU_ASSERT_TRUE(netm_is_network_ready());
}

void test_net_manager_is_ready(void)
{
    DM_LOGI("*** test_net_manager_is_ready {");
    network_conditions_t event;
    event.net_feature = NETWORK_WIFI_CONNECTED;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    event.net_feature = NETWORK_CELLULAR_CONNECTED;
    event.enabled = true;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    event.net_feature = NETWORK_DATA_ROAMING;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    event.net_feature = NETWORK_ACTIVE_VOICE_CALL;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    CU_ASSERT_EQUAL(netm_is_network_ready(),1);
    event.net_feature = NETWORK_DATA_ROAMING;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    event.net_feature = NETWORK_ACTIVE_VOICE_CALL;
    event.enabled = true;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    CU_ASSERT_EQUAL(netm_is_network_ready(),0);

    event.net_feature = NETWORK_DATA_ROAMING;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    event.net_feature = NETWORK_ACTIVE_VOICE_CALL;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    CU_ASSERT_EQUAL(netm_is_network_ready(),1);

    event.net_feature = NETWORK_DATA_ROAMING;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    event.net_feature = NETWORK_ACTIVE_VOICE_CALL;
    event.enabled = true;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    CU_ASSERT_EQUAL(netm_is_network_ready(),0);

    event.net_feature = NETWORK_DATA_ROAMING;
    event.enabled = true;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    event.net_feature = NETWORK_ACTIVE_VOICE_CALL;
    event.enabled = false;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    CU_ASSERT_EQUAL(netm_is_network_ready(),0);

    event.net_feature = NETWORK_DATA_ROAMING;
    event.enabled = true;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    event.net_feature = NETWORK_ACTIVE_VOICE_CALL;
    event.enabled = true;
    CU_ASSERT_EQUAL(netm_handler_process_event(event), 0);
    CU_ASSERT_EQUAL(netm_is_network_ready(),0);

    DM_LOGI("*** test_net_manager_is_ready }");
}

/*
 * Test emulates changes of a roaming state
 */
void test_net_manager_set_roaming(void)
{
   network_conditions_t event;
   event.net_feature = NETWORK_DATA_ROAMING;
   event.enabled = true;
   CU_ASSERT_EQUAL(netm_handler_process_event(event),0);
}


int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if (NULL == CU_add_test(pSuite, "test init net manager",
                           test_init_net_manager)) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (NULL == CU_add_test(pSuite, "test net manager process event",
                           test_net_manager_process_event)) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (NULL == CU_add_test(pSuite, "test net manager set states",
                           test_net_manager_set_states)) {
      CU_cleanup_registry();
      return CU_get_error();
   }
   if (NULL == CU_add_test(pSuite, "test net manager set roaming",
                           test_net_manager_set_roaming)) {
      CU_cleanup_registry();
      return CU_get_error();
   }
   if (NULL == CU_add_test(pSuite, "test net manager isready",
                           test_net_manager_is_ready)) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (NULL == CU_add_test(pSuite, "test stop net manager",
                           test_stop_net_manager)) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_SILENT);
   CU_basic_run_tests();
   unsigned int failedTests = CU_get_number_of_tests_failed();
   if(failedTests > 0) {
       printf("Failure list:");
       CU_basic_show_failures(CU_get_failure_list());
       printf("\n");
   }
   CU_ErrorCode errorResult = CU_get_error();
   CU_cleanup_registry();
   return errorResult == CUE_SUCCESS ? failedTests : errorResult;
}
