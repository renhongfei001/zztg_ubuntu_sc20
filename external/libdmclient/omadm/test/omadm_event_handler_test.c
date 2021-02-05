/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "omadm_event_handler.h"

typedef int (*pal_notify_on_omadm_event_t)(process_omadm_event_t);
static pal_notify_on_omadm_event_t pal_notify_on_omadm_event_fn = NULL;

int omadm_event_handler_init_suite(void)
{
    remove("./data/fumo.state.store");
    return 0;
}

/*
 * The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int omadm_event_handler_clean_suite(void)
{
    remove("./data/fumo.state.store");
    return 0;
}

static void * omadm_event_handler_test_dlopen(void)
{
    void * pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    if (!pal_handle) {
        fprintf(stderr, "%s\n", dlerror());
    }
    return pal_handle;
}

/*
 * Test initialization of event handler
 */
void omadm_event_handler_test_init(void)
{
    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL(omadm_event_handler_init(), RESULT_SUCCESS);
}

/*
 * Test creates event package and emulates notification callback
 */
void omadm_event_handler_test_event(void)
{
    remove("./data/fumo.state.store");
    void * pal_handle = omadm_event_handler_test_dlopen();
    if (pal_handle != NULL) {
        pal_notify_on_omadm_event_fn = dlsym(pal_handle,
                "pal_notify_on_omadm_event");
    }
    if (pal_notify_on_omadm_event_fn != NULL)
        CU_ASSERT_EQUAL(pal_notify_on_omadm_event_fn(
                &omadm_event_handler_process_event), RESULT_SUCCESS);
}

/*
 * Test creates SU cancel event and emulates notification callback
 */
void omadm_event_handler_test_sucancel(void)
{
    remove("./data/fumo.state.store");
    printf("TEST: Su cancel \n");
    buffer_t buff;
    buff.buffer = strdup("Anything");
    buff.len = strlen(buff.buffer);
    CU_ASSERT_EQUAL(omadm_event_handler_process_event(SU_CANCEL, &buff),
                    RESULT_SUCCESS);
    if(buff.buffer)
        free(buff.buffer);
}

/*
 * Test processing alert
 */
void omadm_event_handler_test_alert(void)
{
    remove("./data/fumo.state.store");
    alert_t * alert = malloc(sizeof(alert_t));
    if (alert != NULL) {
        /// \todo : make this test more sensible
        alert->mo = strdup("FUMO TEST");
        omadm_mo_event_t *event = malloc(sizeof(omadm_mo_event_t));
        if(event) {
            event->dmclt_item.format = strdup("application/vnd.syncml.dm+xml");
            event->correlator = strdup("correlator");
            event->dmclt_item.data = strdup("event description");
            event->dmclt_item.source = strdup("node name");
            event->dmclt_item.target = strdup("com.vzwdmserver");
            event->dmclt_item.type = strdup("urn_url");
            alert->alert_data =(char *)event;
            CU_ASSERT_EQUAL(omadm_event_handler_process_alert(alert),
                    RESULT_SUCCESS);
        }
    }
}

int main()
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("Suite_1", omadm_event_handler_init_suite,
            omadm_event_handler_clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if (NULL == CU_add_test(pSuite, "omadm_event_handler init",
            omadm_event_handler_test_init)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "omadm_event_handler event",
            omadm_event_handler_test_event)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "omadm_event_handler alert",
            omadm_event_handler_test_alert)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "omadm_event_handler SU cancel",
            omadm_event_handler_test_sucancel)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_SILENT);
    CU_basic_run_tests();
    unsigned int failedTests = CU_get_number_of_tests_failed();
    if (failedTests > 0) {
        printf("Failure list:");
        CU_basic_show_failures(CU_get_failure_list());
        printf("\n");
    }
    CU_ErrorCode errorResult = CU_get_error();
    CU_cleanup_registry();
    return errorResult == CUE_SUCCESS ? failedTests : errorResult;
}
