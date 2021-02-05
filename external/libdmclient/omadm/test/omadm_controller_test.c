/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdio.h>
#include <string.h>
#include "omadm_controller.h"
#include "pal.h"

#define FORMAT_TYPE_XML 0
#define FORMAT_TYPE_WBXML 1
#define SERVER_ID "com.vzwdmserver"

#define SESSION_ID 1
extern int event_handler_callback(omadm_mo_event_t *event);

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

/** Test OMADM controller start session
 * @brief omadm_controller_test_start_session
 */
void omadm_controller_test_start_session(void)
{
    omadm_env_t env;
    env.format_type = FORMAT_TYPE_XML;
    env.server_id = strdup(SERVER_ID);
    env.session_id = SESSION_ID;
    CU_ASSERT_EQUAL(omadm_controller_setup_session(&env),DMCLT_ERR_NONE);
    omadm_request_t req;
    req.alert = NULL;
    req.package0 = NULL;
    req.type = PACKAGE0;
    CU_ASSERT_EQUAL(omadm_controller_handle_request((void *) &req),
            DMCLT_ERR_USAGE);
    req.alert = NULL;
    req.package0 = NULL;
    req.type = USER_SYS_UPDATE;
    CU_ASSERT_EQUAL(omadm_controller_handle_request((void *) &req),
            DMCLT_ERR_END);
}

/** Test OMADM controller start session wbxml
 * @brief omadm_controller_test_start_session
 */
void omadm_controller_test_start_session_wbxml(void)
{
    omadm_env_t env;
    env.format_type = FORMAT_TYPE_WBXML;
    env.server_id = strdup(SERVER_ID);
    env.session_id = SESSION_ID;
    CU_ASSERT_EQUAL(omadm_controller_setup_session(&env),DMCLT_ERR_NONE);
    omadm_request_t req;
    req.alert = NULL;
    req.package0 = NULL;
    req.type = PACKAGE0;
    CU_ASSERT_EQUAL(omadm_controller_handle_request((void *) &req),
            DMCLT_ERR_INTERNAL);
}

/** Test OMADM controller cancel session
 * @brief omadm_controller_test_interrupt_session
 */
void omadm_controller_test_interrupt_session(void)
{
    CU_ASSERT_EQUAL(omadm_controller_interrupt_session(),DMCLT_ERR_NONE);
}

/** Test OMADM controller callbacks
 * @brief omadm_controller_test_callbacks
 */
void omadm_controller_test_callbacks(void)
{
    omadm_mo_ft_event_handler g_dmclient_event_handler = NULL;
    omadm_mo_event_t * event;
    omadm_env_t env;
    env.format_type = FORMAT_TYPE_XML;
    env.server_id = strdup(SERVER_ID);
    env.session_id = SESSION_ID;
    CU_ASSERT_EQUAL(omadm_controller_setup_session(&env),DMCLT_ERR_NONE);
    omadm_request_t req;
    req.alert = NULL;
    req.package0 = NULL;
    req.type = USER_SYS_UPDATE;
    CU_ASSERT_EQUAL(omadm_controller_handle_request((void *) &req),
            DMCLT_ERR_END);
    g_dmclient_event_handler = event_handler_callback;
    CU_ASSERT_NOT_EQUAL(g_dmclient_event_handler,NULL);

    event = malloc(sizeof(omadm_mo_event_t));
    if(event) {
        event->dmclt_item.format = strdup("application/vnd.syncml.dm+xml");
        event->correlator = strdup("correlator");
        event->dmclt_item.data = strdup("event description");
        event->dmclt_item.source = strdup("node name");
        event->dmclt_item.target = strdup("com.vzwdmserver");
        event->dmclt_item.type = strdup("urn_url");
        CU_ASSERT_EQUAL(g_dmclient_event_handler(event),RESULT_SUCCESS);
    }

}

/**
 * Test OMADM controller start with alert
 */
void omadm_controller_test_startalert()
{
    omadm_env_t env;
    env.format_type = FORMAT_TYPE_XML;
    env.server_id = strdup(SERVER_ID);
    env.session_id = SESSION_ID;
    CU_ASSERT_EQUAL(omadm_controller_setup_session(&env),DMCLT_ERR_NONE);
    omadm_request_t req;
    req.alert = malloc(sizeof(alert_t));
    if (req.alert != NULL) {
        memset(req.alert,0,sizeof(alert_t));
        req.alert->alert_data = strdup(OMADM_ALERT_SESSION_ABORT);
    }
    req.package0 = NULL;
    req.type = SESSION_ALERT;
    CU_ASSERT_EQUAL(omadm_controller_handle_request((void *) &req),
            DMCLT_ERR_END);
}

/**
 * It adds tests suuites and test cases for OMADM controller
 * @return 0 if success, 1 otherwise
 */
int omadm_controller_add_test_suite(void)
{
    CU_TestInfo tests[] = {
         {"OMADM controller test start session",
           omadm_controller_test_start_session },
         {"OMADM controller test interrupt session",
           omadm_controller_test_interrupt_session },
         {"OMADM controller test start session wbxml",
           omadm_controller_test_start_session_wbxml },
         {"OMADM controller test callbacks",
          omadm_controller_test_callbacks },
         {"OMADM controller test alert",
          omadm_controller_test_startalert},
         CU_TEST_INFO_NULL,
     };

     CU_SuiteInfo suites[] = {
         { "OMADM_test_suite", NULL, NULL, tests },
         CU_SUITE_INFO_NULL,
     };

     return ( CU_register_suites( suites ) == CUE_SUCCESS ? 0 : 1 );
}

int main(){
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

    /* adds and initialize OMADM controller test cases */
    if( omadm_controller_add_test_suite() )
    {
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

