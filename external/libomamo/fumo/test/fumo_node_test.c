/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "fumo.h"

omadm_mo_interface_t* g_interface = NULL;
void *context = NULL;

extern int gui_supported;

int init_suite()
{
    if(NULL == (g_interface=omadm_get_mo_interface()))
        return 1;
    if(NULL == g_interface->initFunc)
        return 1;
    if(MO_ERROR_NONE != g_interface->initFunc(&context))
        return 1;

    return 0;
};

int clean_suite()
{
    if(g_interface) {
        if(NULL != g_interface->closeFunc)
            g_interface->closeFunc(context);
        if(g_interface->base_uri) {
            free(g_interface->base_uri);
        }
        free(g_interface);
    }
    return 0;
};


/** @todo write test_case_find_node */
void test_case_find_node(void)
{
    // int find_node(const char *uri,
    //              fumo_node **node)
    char uri1[] = "./ManagedObjects/FUMO/Download";
    char uri2[] = "";
    CU_ASSERT_EQUAL( find_node(uri1, NULL), MO_ERROR_COMMAND_FAILED);
    CU_ASSERT_EQUAL(find_node(uri2, NULL), MO_ERROR_COMMAND_FAILED);
};

void test_case_on_gui_response(void)
{
    pal_fumo_gui_user_reply_param data;
    on_gui_response(&data);
}

void test_case_clean_gui_message_descriptor(void)
{
    pal_fumo_gui_message_descriptor md;
    md.header_text = strdup("Text");
    md.message_text = strdup("Text");
    md.hyper_link_caption = strdup("Text");
    md.hyper_link = strdup("Text");
    md.progress_bar_caption = strdup("Text");
    md.install_param = strdup("Text");
    md.severity = strdup("Text");
    clean_gui_message_descriptor(&md);
    CU_ASSERT_PTR_NULL(md.header_text);
    CU_ASSERT_PTR_NULL(md.message_text);
    CU_ASSERT_PTR_NULL(md.hyper_link_caption);
    CU_ASSERT_PTR_NULL(md.hyper_link);
    CU_ASSERT_PTR_NULL(md.progress_bar_caption);
    CU_ASSERT_PTR_NULL(md.install_param);
    CU_ASSERT_PTR_NULL(md.severity);
}

void test_case_node_command_exec(void)
{
    //int node_command_exec(struct fumo_node* pNode,
    //                    const char * cmdData, const char * correlator)
    fumo_node n, *pn = NULL;
    char *cmdData = NULL;
    char *correlator = NULL;
    void *threadRC = NULL;
    char *badUri = "badUri";

    memset(&n, 0, sizeof(n));

    CU_ASSERT_EQUAL(node_command_exec(NULL, cmdData, correlator),
        MO_ERROR_COMMAND_FAILED);

    g_fumo_work_thread_id = 1;
    n.uri_ = badUri;
    CU_ASSERT_EQUAL(node_command_exec(&n, cmdData, correlator),
        FUMO_RC_UPDATE_DEFERRED);

    g_fumo_work_thread_id = 0;
    CU_ASSERT_EQUAL(node_command_exec(&n, cmdData, correlator),
       MO_ERROR_NOT_ALLOWED);

    CU_ASSERT_EQUAL(find_node("./ManagedObjects/FUMO/Download", &pn),
        MO_ERROR_NONE);
    threadRC = NULL;
    CU_ASSERT_EQUAL(node_command_exec(pn, cmdData, correlator),
        FUMO_RC_ACCEPTED_FOR_PROCESSING);
    CU_ASSERT_FALSE( pthread_join(g_fumo_work_thread_id, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    CU_ASSERT_EQUAL(find_node("./ManagedObjects/FUMO/Update", &pn),
        MO_ERROR_NONE);
    g_fumo_work_thread_id = 0;
    threadRC = NULL;
    fumo_storage_set_pkg_name("packet");
    CU_ASSERT_EQUAL(node_command_exec(pn, cmdData, correlator),
       FUMO_RC_ACCEPTED_FOR_PROCESSING);
    CU_ASSERT_FALSE( pthread_join(g_fumo_work_thread_id, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    CU_ASSERT_EQUAL(find_node("./ManagedObjects/FUMO/DownloadAndUpdate", &pn),
        MO_ERROR_NONE);
    g_fumo_work_thread_id = 0;
    threadRC = NULL;
    fumo_storage_set_pkg_name("packet");
    CU_ASSERT_EQUAL(node_command_exec(pn, cmdData, correlator),
       FUMO_RC_ACCEPTED_FOR_PROCESSING);
    CU_ASSERT_FALSE( pthread_join(g_fumo_work_thread_id, &threadRC) );

    CU_ASSERT_EQUAL( threadRC, (void*)0 );
};

void test_case_node_command_exec_gui_download_failed(void)
{
    //int node_command_exec(struct fumo_node* pNode,
    //                    const char * cmdData, const char * correlator)
    fumo_node n, *pn = NULL;
    char *cmdData = NULL;
    char *correlator = NULL;
    void *threadRC = NULL;
    char *badUri = "badUri";

    gui_supported = 1;

    memset(&n, 0, sizeof(n));

    CU_ASSERT_EQUAL(node_command_exec(NULL, cmdData, correlator),
        MO_ERROR_COMMAND_FAILED);

    g_fumo_work_thread_id = 1;
    n.uri_ = badUri;
    CU_ASSERT_EQUAL(node_command_exec(&n, cmdData, correlator),
        FUMO_RC_UPDATE_DEFERRED);

    g_fumo_work_thread_id = 0;
    CU_ASSERT_EQUAL(node_command_exec(&n, cmdData, correlator),
       MO_ERROR_NOT_ALLOWED);

    CU_ASSERT_EQUAL(find_node("./ManagedObjects/FUMO/Download", &pn),
        MO_ERROR_NONE);
    threadRC = NULL;
    CU_ASSERT_EQUAL(node_command_exec(pn, cmdData, correlator),
        FUMO_RC_ACCEPTED_FOR_PROCESSING);
    CU_ASSERT_FALSE( pthread_join(g_fumo_work_thread_id, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    CU_ASSERT_EQUAL(find_node("./ManagedObjects/FUMO/DownloadAndUpdate", &pn),
        MO_ERROR_NONE);
    g_fumo_work_thread_id = 0;
    threadRC = NULL;
    fumo_storage_set_pkg_name("packet");
    CU_ASSERT_EQUAL(node_command_exec(pn, cmdData, correlator),
       FUMO_RC_ACCEPTED_FOR_PROCESSING);
    CU_ASSERT_FALSE( pthread_join(g_fumo_work_thread_id, &threadRC) );

    CU_ASSERT_EQUAL( threadRC, (void*)0 );
};

void test_case_get_string_node_format(void)
{
    //int get_string_node_format(fumo_node_format nf, char **format)
    fumo_node_format nf = efnf_Unknown;
    char *format = NULL;

    nf = efnf_Unknown;

    CU_ASSERT_TRUE(get_string_node_format(nf, NULL));

    format = NULL;
    CU_ASSERT_FALSE(get_string_node_format(nf, &format));
    CU_ASSERT_PTR_NOT_NULL(format);
    CU_ASSERT_FALSE(strcmp(format, "unknown"));
    free(format);

    nf = efnf_Node;
    format = NULL;
    CU_ASSERT_FALSE(get_string_node_format(nf, &format));
    CU_ASSERT_PTR_NOT_NULL(format);
    CU_ASSERT_FALSE(strcmp(format, "node"));
    free(format);

    nf = efnf_Char;
    format = NULL;
    CU_ASSERT_FALSE(get_string_node_format(nf, &format));
    CU_ASSERT_PTR_NOT_NULL(format);
    CU_ASSERT_FALSE(strcmp(format, "chr"));
    free(format);

    nf = efnf_Bin;
    format = NULL;
    CU_ASSERT_FALSE(get_string_node_format(nf, &format));
    CU_ASSERT_PTR_NOT_NULL(format);
    CU_ASSERT_FALSE(strcmp(format, "bin"));
    free(format);

    nf = efnf_Int;
    format = NULL;
    CU_ASSERT_FALSE(get_string_node_format(nf, &format));
    CU_ASSERT_PTR_NOT_NULL(format);
    CU_ASSERT_FALSE(strcmp(format, "int"));
    free(format);
};

void test_case_get_string_node_type(void)
{
    //int get_string_node_type(fumo_node_format nf, char **type)
    fumo_node_format nf = efnf_Unknown;
    char *type = NULL;

    nf = efnf_Unknown;
    type = NULL;
    CU_ASSERT_FALSE(get_string_node_type(nf, &type));
    CU_ASSERT_PTR_NOT_NULL(type);
    CU_ASSERT_FALSE(strcmp(type, "text/plain"));
    free(type);

    nf = efnf_Node;
    type = NULL;
    CU_ASSERT_FALSE(get_string_node_type(nf, &type));
    CU_ASSERT_PTR_NULL(type);

    nf = efnf_Char;
    type = NULL;
    CU_ASSERT_FALSE(get_string_node_type(nf, &type));
    CU_ASSERT_PTR_NOT_NULL(type);
    CU_ASSERT_FALSE(strcmp(type, "text/plain"));
    free(type);

    nf = efnf_Bin;
    type = NULL;
    CU_ASSERT_FALSE(get_string_node_type(nf, &type));
    CU_ASSERT_PTR_NOT_NULL(type);
    CU_ASSERT_FALSE(strcmp(type, "text/plain"));
    free(type);

    nf = efnf_Int;
    type = NULL;
    CU_ASSERT_FALSE(get_string_node_type(nf, &type));
    CU_ASSERT_PTR_NOT_NULL(type);
    CU_ASSERT_FALSE(strcmp(type, "text/plain"));
    free(type);
};


void test_case_get_node_value()
{
    //int get_node_value(const fumo_node* n,

    fumo_node *pn = NULL, n;
    unsigned int size = 0;
    char* data = NULL;

    CU_ASSERT_EQUAL(get_node_value( NULL, NULL, NULL ),
        MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL(get_node_value(pn, &size, &data), MO_ERROR_COMMAND_FAILED);

    memset(&n, 0, sizeof(n));
    n.uri_ = FUMO_BASE_URI "/" FUMO_URI_STATE;
    CU_ASSERT_EQUAL(get_node_value(&n, &size, &data), MO_ERROR_NONE);
    CU_ASSERT_DOUBLE_EQUAL(size, 3, 1);
    CU_ASSERT_EQUAL(strlen(data), size);
}

void test_case_set_node_value(void)
{
    //int set_node_value(fumo_node* n,
    //               const unsigned int size, const char* data)
    g_fumo_work_thread_id = 1;
    CU_ASSERT_EQUAL(set_node_value(NULL, 0, NULL), FUMO_RC_UPDATE_DEFERRED);
    g_fumo_work_thread_id = 0;
}


void test_case_wifi_nodes(void)
{
    fumo_node n;
    unsigned int size = 0;
    char* data = NULL;
    char tmp_buff[256];

    memset(&n, 0, sizeof(n));

    remove(MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE);

    n.uri_ = FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY;
    CU_ASSERT_EQUAL(get_node_value(&n, &size, &data), MO_ERROR_NONE);
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, data, size);
    CU_ASSERT_EQUAL(strcasecmp(tmp_buff, "false"), 0);
    free(data);

    data = "true";
    size = strlen(data);
    CU_ASSERT_EQUAL(set_node_value(&n, size, data), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(get_node_value(&n, &size, &data), MO_ERROR_NONE);
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, data, size);
    CU_ASSERT_EQUAL(strcasecmp(tmp_buff, "true"), 0);
    free(data);

    data = "false";
    size = strlen(data);
    CU_ASSERT_EQUAL(set_node_value(&n, size, data), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(get_node_value(&n, &size, &data), MO_ERROR_NONE);
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, data, size);
    CU_ASSERT_EQUAL(strcasecmp(tmp_buff, "false"), 0);
    free(data);

    n.uri_ = FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLYTIMER;
    CU_ASSERT_EQUAL(get_node_value(&n, &size, &data), MO_ERROR_NONE);
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, data, size);
    CU_ASSERT_EQUAL(strcasecmp(tmp_buff, "23"), 0);
    free(data);

    data = "42";
    size = strlen(data);
    CU_ASSERT_EQUAL(set_node_value(&n, size, data), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(get_node_value(&n, &size, &data), MO_ERROR_NONE);
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, data, size);
    CU_ASSERT_EQUAL(strcasecmp(tmp_buff, "42"), 0);
    free(data);

    data = "0";
    size = strlen(data);
    CU_ASSERT_EQUAL(set_node_value(&n, size, data), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(get_node_value(&n, &size, &data), MO_ERROR_NONE);
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, data, size);
    CU_ASSERT_EQUAL(strcasecmp(tmp_buff, "0"), 0);
    free(data);

    remove(MO_WORK_PATH FUMO_STATE_DIR FUMO_STATE_FILE);
}


int main()
{
    CU_pSuite suite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    suite = CU_add_suite("fumo_nodes_suite", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_find_node",
            test_case_find_node)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_on_gui_response",
                                    test_case_on_gui_response)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_clean_gui_message_descriptor",
            test_case_clean_gui_message_descriptor)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_node_command_exec",
            test_case_node_command_exec)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_get_string_node_format",
            test_case_get_string_node_format)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_get_string_node_type",
            test_case_get_string_node_type)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_get_node_value",
            test_case_get_node_value)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_set_node_value",
            test_case_set_node_value)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_node_wifi_nodes",
            test_case_wifi_nodes)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_node_command_exec_gui_download_failed",
            test_case_node_command_exec_gui_download_failed)) {
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
