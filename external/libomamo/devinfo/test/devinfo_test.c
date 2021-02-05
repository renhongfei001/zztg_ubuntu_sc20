/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <dirent.h>
#include <dlfcn.h>

#include <stdio.h>
#include <string.h>

#include "dm_logger.h"
#include "mo_error.h"
#include "mo_omadmtree.h"
#include "pal.h"
#include "plugin_utils.h"

/* Pointer to the file used by the tests. */
static omadm_mo_interface_t * _iface = NULL;
static void* palHandle = NULL;

/**
 * Initializes test suite
 *
 * Function loads pal from the pal directory
 * and loads plugin for test
 * @return 0 if success, -1 otherwise
 */
int init_suite(void)
{
    // load pal from the pal directory
    palHandle = dlopen(PAL_INSTALL_DIR"/"PAL_LIB_NAME, RTLD_LAZY);

    //load plugin for test
    _iface = omadm_get_mo_interface();
    if (NULL == _iface) {
        return -1;
    } else {
        return 0;
    }
}

/** The suite cleanup function.
 *
 * @return 0 on success, non-zero otherwise.
 */
int clean_suite(void)
{
    if (NULL != _iface) {
        free(_iface);
    }

    if (NULL != palHandle) {
        dlclose(palHandle);
    }
    _iface = NULL;
    return 0;
}

/** Data initialization test
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 */
void test_init_func(void)
{
   void * Data = NULL;
   if (NULL != _iface) {
       CU_ASSERT_EQUAL(_iface->initFunc(&Data),MO_ERROR_NONE);
   }
}

/** Close plugin test
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 */
void test_close_func(void)
{
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   CU_ASSERT_EQUAL(_iface->initFunc(&Data),MO_ERROR_NONE);
   _iface->closeFunc(&Data);
}

/** Test for getting leaf
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 */
void test_get_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./DevInfo/DevId";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

/** Test for getting leaf
 *
 * precondition: none
 * MO_ERROR_NONE code should be returned
 */
void test_value_get_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./DevInfo/Ext/ICCID";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_is_node_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./DevInfo/DevId";

    void * Data = NULL;
    omadmtree_node_kind_t node_type;
    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->isNodeFunc(node.uri, &node_type, Data), MO_ERROR_NONE);
    }
}

/** Test for getting node
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 */
void test_get_node_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./DevInfo";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

/** Test for setting node
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 **/
void test_set_func(void)
{
   dmtree_node_t node;
   memset(&node, 0, sizeof(dmtree_node_t));
   node.uri = "./DevInfo/Ext/ConfigurationVer";
   node.data_buffer = "01230123";
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
      CU_ASSERT_EQUAL(_iface->setFunc(&node, Data), MO_ERROR_NOT_ALLOWED);
   }
}

/** getACL test
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 **/
void test_get_acl(void)
{
   char *value;
   char* uri = "./DevInfo";

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->getACLFunc(uri, &value, Data), MO_ERROR_NONE);
   }
}

/** findUrn test
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 **/
void test_find_urn(void)
{

   char *urn = "urn:oma:mo:oma-dm-devinfo:1.0";

   char** uri = NULL;

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->findURNFunc(urn, &uri, Data), MO_ERROR_NONE);
   }
}

/**
 * It adds tests suites and test cases for DMACC
 * @return 0 if success, 1 otherwise
 */
int main(){

    CU_pSuite suite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    suite = CU_add_suite("DevInfoSuite", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "init devinfo",
            test_init_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "findUrn devinfo",
            test_find_urn)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "getACL devinfo",
            test_get_acl)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "get node devinfo",
            test_get_node_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "get leaf devinfo",
            test_get_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "get value devinfo",
            test_value_get_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "set devinfo",
            test_set_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "close plugin devinfo",
            test_close_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "isnode",
            test_is_node_func)) {
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
