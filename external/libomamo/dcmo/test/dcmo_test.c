/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <dirent.h>
#include <dlfcn.h>

#include <stdio.h>
#include <string.h>

#include "mo_omadmtree.h"
#include "mo_error.h"
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
    if (NULL == _iface){
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
 * MO_ERROR_NONE code should be returned
 */
void test_init_func(void)
{
    void * Data = NULL;
   if (NULL != _iface) {
       CU_ASSERT_EQUAL(_iface->initFunc(&Data),MO_ERROR_NONE);
   }
}

/** Test for getting node
 *
 * precondition: none
 * MO_ERROR_NONE code should be returned
 */
void test_get_node_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DCMO";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == MO_ERROR_NONE ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}


/** Test for executing node
 *
 * precondition: none
 * MO_ERROR_NONE code should be returned
 **/
void test_exec_enable_func(void)
{
   char* uri = "./ManagedObjects/DCMO/VOLTE/FeatureStatus/VLT/Setting/Operations/Enable";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == MO_ERROR_NONE ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NONE);
   }
}

void test_get_enable_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DCMO/VOLTE/FeatureStatus/VLT/Setting/Enabled";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == MO_ERROR_NONE ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "True");
    }
}

void test_exec_disable_func(void)
{
   char* uri = "./ManagedObjects/DCMO/VOLTE/FeatureStatus/VLT/Setting/Operations/Disable";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == MO_ERROR_NONE ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NONE);
   }
}

void test_get_disable_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DCMO/VOLTE/FeatureStatus/VLT/Setting/Enabled";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == MO_ERROR_NONE ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "False");
    }
}


void test_not_allowed_exec_func(void)
{
   char* uri = "./ManagedObjects/DCMO/VOLTE/FeatureStatus/VLT/Setting/Enabled";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == MO_ERROR_NONE ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NOT_ALLOWED);
   }
}

void test_not_find_get_func(void)
{
   dmtree_node_t node;
   memset(&node, 0, sizeof(dmtree_node_t));
   node.uri = "./ManagedObjects/DCMO/test";

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == MO_ERROR_NONE ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NOT_FOUND);
   }
}

void test_not_find_set_func(void)
{
   dmtree_node_t node;
   memset(&node, 0, sizeof(dmtree_node_t));
   node.uri = "./ManagedObjects/DCMO/test";

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == MO_ERROR_NONE ){
        CU_ASSERT_EQUAL(_iface->setFunc(&node, Data), MO_ERROR_NOT_ALLOWED);
   }
}

void test_get_bool_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DCMO/VOLTE/FeatureStatus/LVC/Setting/Enabled";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == MO_ERROR_NONE ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "False");
    }
}

void test_set_not_found_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DCMO/VOLTE/test";
    node.data_buffer = "50";
    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == MO_ERROR_NONE ){
        CU_ASSERT_EQUAL(_iface->setFunc(&node, Data), MO_ERROR_NOT_ALLOWED);
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

    suite = CU_add_suite("DcmoCoreSuite", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "init dcmo",
            test_init_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "get dcmo",
            test_get_node_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "not find get dcmo",
            test_not_find_get_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "not find set dcmo",
            test_not_find_set_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "exec enable dcmo",
            test_exec_enable_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "get enable dcmo",
            test_get_enable_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "exec disable dcmo",
            test_exec_disable_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "get enable dcmo",
            test_get_disable_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "exec dcmo not all",
            test_not_allowed_exec_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_set_not_found_func",
            test_set_not_found_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_bool_func",
            test_get_bool_func)) {
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
