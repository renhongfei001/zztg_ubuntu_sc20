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
       CU_ASSERT_EQUAL(_iface->initFunc(&Data),PLUGIN_SUCCESS);
   }
}

/** Test for getting leaf
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 **/
void test_get_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./DevDetail/OEM";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), RESULT_SUCCESS);
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
    node.uri = "./DevDetail";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), PLUGIN_SUCCESS);
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
   char* uri = "./DevDetail";

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->getACLFunc(uri, &value, Data), PLUGIN_SUCCESS);
   }
}

/** findUrn test
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 **/
void test_find_urn(void)
{

   char *urn = "urn:oma:mo:oma-dm-devdetail:1.0";

   char** uri = NULL;

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->findURNFunc(urn, &uri, Data), PLUGIN_SUCCESS);
   }
}

/**
 * It adds tests suites and test cases for DMACC
 * @return 0 if success, 1 otherwise
 */
int main(){
   CU_TestInfo testsCore[] = {
       {"init devdetail", test_init_func},
       {"findUrn devdetail", test_find_urn},
       {"getACL devdetail", test_get_acl},
       {"get leaf devdetail", test_get_func},
       {"get node devdetail", test_get_node_func},
       CU_TEST_INFO_NULL,
   };

   CU_SuiteInfo suites[] = {
       {"DmAccCoreSuite", init_suite,clean_suite,testsCore},
       CU_SUITE_INFO_NULL,
   };

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   if( CU_register_suites( suites ) != CUE_SUCCESS ) {
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
