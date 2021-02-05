/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "dm_logger.h"
#include "mo_error.h"
#include "mo_omadmtree.h"
#include "plugin_utils.h"
#include "scm.h"
#include "scm_error.h"

static omadm_mo_interface_t* g_interface = 0;

#define TEST_URI_PKG        "./SCM/download/Testapk"
#define TEST_URI_PKG_ID     "./SCM/download/Testapk/PkgID"
#define TEST_URI_PKG_URL    "./SCM/download/Testapk/PkgURL"
#define TEST_URI_OPERATION  "./SCM/download/Testapk/Operations"
#define TEST_URI_COMMAND_DL "./SCM/download/Testapk/Operations/Download"
#define TEST_DL_URI ""

/**
 * Initializes test suite
 *
 * Function loads pal from the pal directory
 * and loads plugin for test
 * @return 0 if success, -1 otherwise
 */
int init_suite(void)
{
    g_interface = omadm_get_mo_interface();
    return (g_interface != 0 ? 0 : 1);
}

/** The suite cleanup function.
 *
 * @return 0 on success, non-zero otherwise.
 */
int clean_suite(void)
{
    g_interface->closeFunc(NULL);
    if(g_interface) {
        if(g_interface->base_uri) {
            free(g_interface->base_uri);
        }
        free(g_interface);
    }
    return 0;
}

/**
 * @brief it prepare and execute download command for Testapk package
 */
void test_case_download()
{
    void *data = NULL;
    DM_LOGI("SCOMO TEST: test_case_download");

    if(g_interface != NULL)
        g_interface = omadm_get_mo_interface();

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    // Init plugin
    CU_ASSERT_EQUAL( g_interface->initFunc(&data),
        MO_ERROR_NONE );

    // Add nodes for Testapk
    // add interior nodes:
    dmtree_node_t nodeO;
    memset(&nodeO, 0, sizeof(nodeO));
    dmtree_node_t nodeP;
    memset(&nodeP, 0, sizeof(nodeP));
    nodeP.format = "node";
    nodeP.type = NULL;
    nodeP.uri = TEST_URI_PKG;
    nodeP.data_buffer = NULL;
    nodeP.data_size = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    nodeO.uri = TEST_URI_PKG;
    int ret = g_interface->getFunc(&nodeO,data);
    if(ret == MO_ERROR_NOT_FOUND) {

        CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);
        CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP,data),MO_ERROR_NONE);
    } else
        DM_LOGI("SCOMO TEST: node %s already exist",TEST_URI_PKG);

    if(nodeO.data_buffer)
        free(nodeO.data_buffer);
    if(nodeO.format)
        free(nodeO.format);
    if(nodeO.type)
        free(nodeO.type);
    memset(&nodeO, 0, sizeof(nodeO));

    nodeP.uri = TEST_URI_OPERATION;
    nodeO.uri = TEST_URI_OPERATION;
    ret = g_interface->getFunc(&nodeO,data);
    if(ret == MO_ERROR_NOT_FOUND) {
        CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);
        CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP,data),MO_ERROR_NONE);
    }
    if(nodeO.data_buffer)
        free(nodeO.data_buffer);
    if(nodeO.format)
        free(nodeO.format);
    if(nodeO.type)
        free(nodeO.type);
    memset(&nodeO, 0, sizeof(nodeO));

    // add leaf nodes:
    nodeP.uri = TEST_URI_PKG_ID;
    nodeP.format = "chr";
    nodeP.type = "text/plain";
    nodeP.data_buffer = "id_001";
    nodeP.data_size = strlen(nodeP.data_buffer)+1;
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP,data),MO_ERROR_NONE);

    nodeP.uri = TEST_URI_PKG_URL;
    nodeP.data_buffer = "https://unknown.com";
    nodeP.data_size = strlen(nodeP.data_buffer)+1;
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP,data),MO_ERROR_NONE);
    // replase
    nodeP.data_buffer = TEST_DL_URI;
    nodeP.data_size = strlen(nodeP.data_buffer)+1;
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP,data),MO_ERROR_NONE);


    nodeP.uri = TEST_URI_COMMAND_DL;
    nodeP.data_buffer = "download_cmd";
    nodeP.data_size = strlen(nodeP.data_buffer)+1;
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP,data),MO_ERROR_NONE);

    // replase
    nodeP.data_buffer = TEST_DL_URI;
    nodeP.data_size = strlen(nodeP.data_buffer)+1;
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP,data),MO_ERROR_NONE);

    // read it
    nodeO.uri = TEST_URI_COMMAND_DL;
    CU_ASSERT_EQUAL(g_interface->getFunc(&nodeO,data),MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(nodeO.data_buffer);
    if(nodeO.data_buffer) {
        DM_LOGI("SCOMO_TEST: nodeO.data_buffer = %s",nodeO.data_buffer);
        CU_ASSERT_STRING_EQUAL(nodeO.data_buffer,TEST_DL_URI);
    }
    if(nodeO.data_buffer)
        free(nodeO.data_buffer);
    if(nodeO.format)
        free(nodeO.format);
    if(nodeO.type)
        free(nodeO.type);
    memset(&nodeO, 0, sizeof(nodeO));

    // execute
    char * cmd_data = "test download";
    char * correlator = "0123456789";
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->execFunc);
    CU_ASSERT_EQUAL(g_interface->execFunc(TEST_URI_COMMAND_DL,
                                          cmd_data,
                                          correlator,
                                          data), MO_ERROR_NONE);

    // close plugin
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    g_interface->closeFunc(data);
}

/**
 * It adds tests suites and test cases for DMACC
 * @return 0 if success, 1 otherwise
 */
int main(){
    CU_pSuite pSuite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    pSuite = CU_add_suite("SCMSuite", init_suite, clean_suite);
    if(0 == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(NULL == CU_add_test(pSuite, "test_case_download",
                            test_case_download)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_SILENT);
    CU_basic_run_tests();
    unsigned int failedTests = CU_get_number_of_tests_failed();
    if(failedTests)
    {
         printf("Failure list:");
         CU_basic_show_failures(CU_get_failure_list());
         printf("\n");
    }
    CU_ErrorCode errorResult = CU_get_error();
    CU_cleanup_registry();
    return errorResult == CUE_SUCCESS ? failedTests : errorResult;
}
