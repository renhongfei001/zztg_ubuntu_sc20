/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <dirent.h>
#include <dlfcn.h>

#include "mo_error.h"
#include "mo_omadmtree.h"
#include "plugin_utils.h"

plugin_tree_node_t test_node [] =
{
    {"/TestMO", "urn:oma:mo:oma-test:1.0",
        OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
        "Get=*", "", NULL, NULL, NULL},
    {"/TestMO/ext",NULL,
        OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
        "Get=*", "", NULL, NULL, NULL},
    {"NULL", NULL, NULL, OMADM_NOT_EXIST, NULL, NULL,NULL,NULL,NULL}
};
/**
 * Initializes test suite
 *
 * Function loads pal from the pal directory
 * and loads plugin for test
 * @return 0 if success, -1 otherwise
 */
int init_suite(void)
{
   return 0;
}

/** The suite cleanup function.
 *
 * @return 0 on success, non-zero otherwise.
 */
int clean_suite(void)
{
   return 0;
}

void test_empty_node(void)
{
   char* uri = "./ManagedObjects/ConnMO/";
   plugin_tree_node_t *test_node = NULL;

   CU_ASSERT_EQUAL(prv_find_node(test_node, uri), PLUGIN_NODE_NOT_EXIST);
}

void test_empty_uri(void)
{
   char* uri = NULL;
   CU_ASSERT_EQUAL(prv_find_node(&test_node, uri), PLUGIN_NODE_NOT_EXIST);
}

void test_find_node(void)
{
   char* uri = "/TestMO";

  CU_ASSERT_EQUAL(prv_find_node(&test_node, uri), PLUGIN_SUCCESS);
}

void test_not_find_node(void)
{
   char* uri = "/TestMO/err";

   CU_ASSERT_EQUAL(prv_find_node(&test_node, uri), PLUGIN_NODE_NOT_EXIST);
}

void test_find_urn(void)
{

   char *urn = "urn:oma:mo:oma-test:1.0";

   char** uri = NULL;

   CU_ASSERT_EQUAL(prv_find_urn(urn, &uri, &test_node), PLUGIN_SUCCESS);
}

void test_get_acl(void)
{
   char *value;
   char* uri = "/TestMO";
   //CU_ASSERT_EQUAL(prv_get_acl_fn(uri, &value, &test_node), PLUGIN_SUCCESS);
}

void test_get_child_list(void)
{
   char* uri = "/TestMO";
   CU_ASSERT_STRING_EQUAL(get_child_list(&test_node,uri),"ext");
}

void test_init_to_current(void)
{
    FILE * file;
    char * way = INIT_DATA_LOCATION;
    char * cur_way = CURRENT_DATA_LOCATION CURRENT_DATA_FILE_NAME;
    copy_init_to_current(way, cur_way);

}

void test_check_path(void)
{
    char * cur_way = "/path";
    check_path(cur_way);
}

void test_parsing_x_storage(void)
{
   char * uri = "./ManagedObjects/DiagMon/Storage/SDCard0";
   char *x_param = NULL;
   CU_ASSERT_EQUAL(parsing_x(uri, &x_param),MO_ERROR_NONE);
   CU_ASSERT_STRING_EQUAL(x_param,"SDCard0");
}

void test_parsing_x_battery(void)
{
   char * uri = "./ManagedObjects/DiagMon/Battery/1";
   char *x_param = NULL;
   CU_ASSERT_EQUAL(parsing_x(uri, &x_param),MO_ERROR_NONE);
   CU_ASSERT_STRING_EQUAL(x_param,"1");
}
void test_parsing_x_error(void)
{
   char * uri = "./ManagedObjects/DiagMon";
   char **x_param = NULL;
   CU_ASSERT_EQUAL(parsing_x(uri, &x_param),MO_ERROR_INCOMPLETE_COMMAND);
}

int main(){
    CU_pSuite suite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    suite = CU_add_suite("Plugin Utils", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_empty_node",
            test_empty_node)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_empty_uri",
            test_empty_uri)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_find_node",
            test_find_node)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_not_find_node",
            test_not_find_node)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_find_urn",
            test_find_urn)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_acl",
            test_get_acl)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_child_list",
            test_get_child_list)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_init_to_current",
            test_init_to_current)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_check_path",
            test_check_path)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_parsing_x_storage",
            test_parsing_x_storage)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_parsing_x_battery",
            test_parsing_x_battery)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_parsing_x_error",
            test_parsing_x_error)) {
        CU_cleanup_registry();
        return CU_get_error();
    }


/* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_SILENT);
    CU_basic_run_tests();
    unsigned int failedTests = CU_get_number_of_tests_failed();
    if(failedTests) {
        printf("Failure list:");
        CU_basic_show_failures(CU_get_failure_list());
        printf("\n");
    }
    CU_ErrorCode errorResult = CU_get_error();
    CU_cleanup_registry();
    return errorResult == CUE_SUCCESS ? failedTests : errorResult;
}
