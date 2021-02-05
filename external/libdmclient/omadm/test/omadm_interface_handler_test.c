/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omadm_interface_handler.h"
#define SERVER_URI "https://google.com"
#define HTTP_METHOD_NOT_ALLOWED 405

int omadm_interface_handler_init_suite(void)
{
    return 0;
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int omadm_interface_handler_clean_suite(void)
{
    return 0;
}

/**
 * Wrong incoming post request parameters test
 * NULL incoming buffers, username and password
 * precondition: none
 * DMCLT_ERR_INTERNAL status should be returned
 */
void omadm_interface_handler_invalid_params_null_buffer_null_login(void)
{
    CU_ASSERT_EQUAL(omadm_interface_handler_post_package(NULL, NULL, true, NULL, NULL),
        DMCLT_ERR_INTERNAL);
}

/**
 * Wrong incoming post request parameters test
 * precondition: none
 * invalid incoming buffer
 * DMCLT_ERR_INTERNAL status should be returned
 */
void omadm_interface_handler_invalid_params_wrong_buffer(void)
{
    // Setup incoming buffer with some stub info
    dmclt_buffer_t *incoming_buffer;
    incoming_buffer = malloc(sizeof(*incoming_buffer));
    CU_ASSERT_PTR_NOT_NULL_FATAL(incoming_buffer);
    memset(incoming_buffer, 0, sizeof(*incoming_buffer));
    char* test_info = "01234567890";
    int size = strlen(test_info);
    incoming_buffer->data = (char *)malloc(size);
    CU_ASSERT_PTR_NOT_NULL_FATAL(incoming_buffer->data);
    memcpy(incoming_buffer->data, test_info, size);
    incoming_buffer->length = size;
    incoming_buffer->auth_type = DMCLT_AUTH_TYPE_HTTP_BASIC;
    char* auth_info = "RpGm0myt/B1ce8tWQ==";
    incoming_buffer->auth_data = malloc(strlen(auth_info)+1);
    memcpy(incoming_buffer->auth_data, auth_info, strlen(auth_info)+1);
    incoming_buffer->auth_data_length = strlen(auth_info);
    incoming_buffer->uri = SERVER_URI;

    // Setup receiving buffer
    dmclt_buffer_t *receiving_buffer;
    receiving_buffer = malloc(sizeof(*receiving_buffer));
    CU_ASSERT_PTR_NOT_NULL_FATAL(receiving_buffer);
    memset(receiving_buffer, 0, sizeof(*receiving_buffer));

    CU_ASSERT_EQUAL(omadm_interface_handler_post_package(incoming_buffer, receiving_buffer,
            true, "admin", "admin"), DMCLT_ERR_ABORT);

    incoming_buffer->auth_type = DMCLT_AUTH_TYPE_HTTP_DIGEST;
    CU_ASSERT_EQUAL(omadm_interface_handler_post_package(incoming_buffer, receiving_buffer,
            true, "admin", "admin"), DMCLT_ERR_ABORT);

    incoming_buffer->auth_type = DMCLT_AUTH_TYPE_HMAC;
    CU_ASSERT_EQUAL(omadm_interface_handler_post_package(incoming_buffer, receiving_buffer,
            true, "admin", "admin"), DMCLT_ERR_ABORT);

    incoming_buffer->auth_type = DMCLT_AUTH_TYPE_UNKNOWN;
    CU_ASSERT_EQUAL(omadm_interface_handler_post_package(incoming_buffer, receiving_buffer,
            true, "admin", "admin"), DMCLT_ERR_ABORT);

    incoming_buffer->auth_type = DMCLT_AUTH_TYPE_SECURID;
    CU_ASSERT_EQUAL(omadm_interface_handler_post_package(incoming_buffer, receiving_buffer,
            true, "admin", "admin"), DMCLT_ERR_ABORT);

    omadm_interface_handler_end_session();
}

int main(){
    CU_TestInfo tests[] = {
        { "omadm_interface_handler_invalid_params_null_buffer_null_login",
                omadm_interface_handler_invalid_params_null_buffer_null_login },
        { "omadm_interface_handler_invalid_params_wrong_buffer",
                omadm_interface_handler_invalid_params_wrong_buffer },
        CU_TEST_INFO_NULL,
    };

    CU_SuiteInfo suites[] = {
        {"OmadmInterfaceHandlerCoreSuite", omadm_interface_handler_init_suite,
            omadm_interface_handler_clean_suite, tests},
        CU_SUITE_INFO_NULL,
    };

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if(CU_register_suites( suites ) != CUE_SUCCESS ) {
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
