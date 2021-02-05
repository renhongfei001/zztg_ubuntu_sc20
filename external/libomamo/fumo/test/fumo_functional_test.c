/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "fumo.h"

#define FUMO_DESCRIPTOR_URL "file://download_descriptor_local.xml"
#define FUMO_PACAKGE_URL "file://VZW_Model_123_456.pdf"

omadm_mo_interface_t* g_interface = 0;

#define FRMWRUPD_FILE "manuf_model_vbase_vtarget"
#define FRMWRUPD_SIZE ((int)1028)

extern int gui_supported;

///////////////////////////////////////////// fumo_core
int init_suite(void)
{
    g_interface = omadm_get_mo_interface();
    return (g_interface != 0 ? 0 : 1);
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite(void)
{
    if(g_interface) {
        if(g_interface->base_uri) {
            free(g_interface->base_uri);
        }
        free(g_interface);
    }
    return 0;
}

void test_case_fumo_download()
{
    printf("\n\n>>>>>>>>>> test_case_fumo_download\n");
    char uriPkgURL[] = "./ManagedObjects/FUMO/Download/PkgURL";
    char uriSeverity[] = "./ManagedObjects/FUMO/Ext/Severity";
    dmtree_node_t nodeP;
    void *context = NULL;
    void *threadRC = NULL;
    struct stat stInp, stOut;
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;
    pthread_t tid = 0;

    remove("./data/fumo.state.store");

    memset(&nodeP, 0, sizeof(nodeP));

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);

    CU_ASSERT_EQUAL(g_interface->initFunc(&context), MO_ERROR_NONE);

    CU_ASSERT_EQUAL(g_interface->isNodeFunc(uriPkgURL, &type, context),
            MO_ERROR_NONE);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    CU_ASSERT_EQUAL(g_interface->isNodeFunc(uriSeverity, &type, context),
            MO_ERROR_NONE);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    nodeP.uri = uriPkgURL;
    nodeP.data_buffer = FUMO_DESCRIPTOR_URL;
    nodeP.data_size = strlen(FUMO_DESCRIPTOR_URL) + 1;

    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);

    nodeP.uri = uriSeverity;
    nodeP.data_buffer = "3";
    nodeP.data_size = 2;

    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);

    int err = g_interface->execFunc("./ManagedObjects/FUMO/Download", "cmdData",
            "correlator", context);
    CU_ASSERT_EQUAL(err, FUMO_RC_ACCEPTED_FOR_PROCESSING);
    tid = g_fumo_work_thread_id;
    CU_ASSERT_FALSE( pthread_join(tid, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    CU_ASSERT_EQUAL( access("data/"DOWNLOAD_DESCRIPTOR_FILE_NAME, R_OK), 0 );
    CU_ASSERT_EQUAL( stat("download_descriptor_local.xml", &stInp), 0 );
    CU_ASSERT_EQUAL( stat("data/"DOWNLOAD_DESCRIPTOR_FILE_NAME, &stOut), 0 );
    CU_ASSERT_EQUAL( stInp.st_size, stOut.st_size );

    g_interface->closeFunc(context);
};

void test_case_fumo_update()
{
    printf("\n\n>>>>>>>>>> test_case_fumo_update\n");
    char uriRoot[] = "./ManagedObjects/FUMO/Update";
    FILE *fd = 0;
    char buff[FRMWRUPD_SIZE];
    char cmdData[] = "cmdData";
    char correlator[] = "correlator";
    void *context = NULL;
    void *threadRC = NULL;
    char *rawline = NULL;
    size_t line_len = 0;
    pthread_t tid = 0;

    pal_download_descriptor_t dd;

    remove("./data/fumo.state.store");

    memset(&dd, 0, sizeof(dd));
    dd.packageName = FRMWRUPD_FILE;
    dd.ddVersion = "ddVersion";
    dd.objectUri = "objectUri";
    dd.size = FRMWRUPD_SIZE;
    dd.type = "type";
    dd.vendor = "vendor";
    dd.description = "description";
    dd.installParam = "installParam";

    dd.preDownloadMessage = "preDownloadMessage";
    dd.postDownloadMessage = "postDownloadMessage";
    dd.preDownloadURL = "preDownloadURL";
    dd.postDownloadURL = "postDownloadURL";
    dd.postUpdateURL = "postUpdateURL";
    dd.postUpdateMessage = "postUpdateMessage";

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->execFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE);

    fd = fopen(MO_WORK_PATH FUMO_STATE_DIR FRMWRUPD_FILE, "w");
    CU_ASSERT_NOT_EQUAL_FATAL(fd, NULL);
    CU_ASSERT_EQUAL_FATAL( fwrite(buff, sizeof(buff), 1, fd), 1 );
    fclose(fd);

    CU_ASSERT_EQUAL( fumo_storage_set_download_descriptor(&dd),
            MO_ERROR_NONE);

    CU_ASSERT_EQUAL( g_interface->execFunc(uriRoot, cmdData, correlator,
                        context), FUMO_RC_ACCEPTED_FOR_PROCESSING);

    tid = g_fumo_work_thread_id;
    CU_ASSERT_FALSE( pthread_join(tid, &threadRC) );

    printf("threadRC = %p\n", threadRC);
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    fd = fopen(MO_WORK_PATH "/data/firmware_update.bin" , "r");
    CU_ASSERT_NOT_EQUAL_FATAL(fd, NULL);
    fseek(fd, 0, SEEK_END);
    CU_ASSERT_EQUAL(ftell(fd), sizeof(buff));//package was reduced on 4 bytes before update
    fclose(fd);

    fd = fopen(MO_WORK_PATH "/data/"PAL_FILENAME_LAST_UPD_DATETIME, "r");
    CU_ASSERT_NOT_EQUAL_FATAL(fd, NULL);
    /** reading MM:DD:YYY */
    CU_ASSERT_NOT_EQUAL_FATAL(
        getdelim(&rawline, &line_len, (int)' ', fd), -1);
    /** reading HH:MM:SS */
    CU_ASSERT_NOT_EQUAL_FATAL(
        getdelim(&rawline, &line_len, (int)' ', fd), -1);
    fclose(fd);

    fd = fopen(MO_WORK_PATH "/data/"PAL_FILENAME_LAST_UPD_FIRMWARE, "r");
    CU_ASSERT_NOT_EQUAL_FATAL(fd, NULL);
    /** reading {FIRMWARE VERSION} */
    CU_ASSERT_NOT_EQUAL_FATAL(
        getline(&rawline, &line_len, fd), -1);
    fclose(fd);
    CU_ASSERT_FALSE(strcmp(rawline, "vtarget"));

    fd = fopen(MO_WORK_PATH "/data/"PAL_FILENAME_FIRMWARE_PACKET_NAME, "r");
    CU_ASSERT_NOT_EQUAL_FATAL(fd, NULL);
    /** reading FIRMWARE_PACKET_NAME */
    CU_ASSERT_NOT_EQUAL_FATAL(
        getline(&rawline, &line_len, fd), -1);
    fclose(fd);
    CU_ASSERT_FALSE(strcmp(rawline, "manuf_model_vbase_vtarget"));

    g_interface->closeFunc(context);
};

/** @todo test_case_fumo_download_update */
void test_case_fumo_download_update()
{
    printf("\n\n>>>>>>>>>> test_case_fumo_download_update\n");
    char uriPkgURL[] = "./ManagedObjects/FUMO/DownloadAndUpdate/PkgURL";
    char uriSeverity[] = "./ManagedObjects/FUMO/Ext/Severity";
    dmtree_node_t nodeP;
    void *context = NULL;
    void *threadRC = NULL;
    struct stat stInp, stOut;
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;
    gui_supported = 0;
    pthread_t tid = 0;

    remove("./data/fumo.state.store");

    memset(&nodeP, 0, sizeof(nodeP));

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);

    CU_ASSERT_EQUAL(g_interface->initFunc(&context), MO_ERROR_NONE);

    CU_ASSERT_EQUAL(g_interface->isNodeFunc(uriPkgURL, &type, context),
            MO_ERROR_NONE);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    CU_ASSERT_EQUAL(g_interface->isNodeFunc(uriSeverity, &type, context),
            MO_ERROR_NONE);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    nodeP.uri = uriPkgURL;
    nodeP.data_buffer = FUMO_DESCRIPTOR_URL;
    nodeP.data_size = strlen(FUMO_DESCRIPTOR_URL) + 1;

    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);

    nodeP.uri = uriSeverity;
    nodeP.data_buffer = "3";
    nodeP.data_size = 2;

    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);
    int err = g_interface->execFunc("./ManagedObjects/FUMO/DownloadAndUpdate",
        "cmdData", "correlator", context);
    tid = g_fumo_work_thread_id;
    CU_ASSERT_EQUAL(err, FUMO_RC_ACCEPTED_FOR_PROCESSING);

    CU_ASSERT_FALSE( pthread_join(tid, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    CU_ASSERT_EQUAL( access("download_descriptor_local.xml", R_OK), 0 );
    CU_ASSERT_EQUAL( access("data/"DOWNLOAD_DESCRIPTOR_FILE_NAME, R_OK), 0 );
    CU_ASSERT_EQUAL( stat("download_descriptor_local.xml", &stInp), 0 );
    CU_ASSERT_EQUAL( stat("data/"DOWNLOAD_DESCRIPTOR_FILE_NAME, &stOut), 0 );
    CU_ASSERT_EQUAL( stInp.st_size, stOut.st_size );

    CU_ASSERT_EQUAL( access("data/VZW_Model_123_456.pdf", R_OK), 0 );
    CU_ASSERT_EQUAL( access("data/firmware_update.bin", R_OK), 0 );
    CU_ASSERT_EQUAL( stat("data/VZW_Model_123_456.pdf", &stInp), 0 );
    CU_ASSERT_EQUAL( stat("data/firmware_update.bin", &stOut), 0 );
    CU_ASSERT_EQUAL( stInp.st_size, stOut.st_size );

    g_interface->closeFunc(context);
};


void test_case_fumo_cancel()
{
    printf("\n\n>>>>>>>>>> test_case_fumo_cancel\n");
    char uriCancel[] = "./ManagedObjects/FUMO/Cancel";
    char uriDownload[] = "./ManagedObjects/FUMO/Download";
    char uriDownloadPkgUrl[] = "./ManagedObjects/FUMO/Download/PkgURL";
    char uriDownloadCancel[] = "./ManagedObjects/FUMO/Download/Cancel";
    char uriUpdate[] = "./ManagedObjects/FUMO/Update";
    char uriUpdateCancel[] = "./ManagedObjects/FUMO/Update/Cancel";
    char uriDAU[] = "./ManagedObjects/FUMO/DownloadAndUpdate";
    char uriDAUCancel[] = "./ManagedObjects/FUMO/DownloadAndUpdate/Cancel";
    char uriDAUPkgUrl[] = "./ManagedObjects/FUMO/DownloadAndUpdate/PkgURL";
    char testSeverity[] = "./ManagedObjects/FUMO/Ext/Severity";
    dmtree_node_t nodeP;
    void *context = NULL;
    char cmdData[] = "cmdData";
    char correlator[] = "correlator";
    void *threadRC = NULL;
    //struct stat stInp, stOut;
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;
    char buff[FRMWRUPD_SIZE];
    FILE *fd = 0;
    pthread_t tid = 0;

    pal_download_descriptor_t dd;
    memset(&dd, 0, sizeof(dd));
    dd.packageName = FRMWRUPD_FILE;
    dd.ddVersion = "ddVersion";
    dd.objectUri = "objectUri";
    dd.size = FRMWRUPD_SIZE;
    dd.requiredInstall = FRMWRUPD_SIZE*3;
    dd.type = "type";
    dd.vendor = "vendor";
    dd.description = "description";
    dd.installParam = "installParam";

    dd.preDownloadMessage = "preDownloadMessage";
    dd.postDownloadMessage = "postDownloadMessage";
    dd.preDownloadURL = "preDownloadURL";
    dd.postDownloadURL = "postDownloadURL";
    dd.postUpdateURL = "postUpdateURL";
    dd.postUpdateMessage = "postUpdateMessage";

    remove("./data/fumo.state.store");

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->execFunc);

    CU_ASSERT_EQUAL(g_interface->initFunc(&context), MO_ERROR_NONE);

    fd = fopen(MO_WORK_PATH "/data/" FRMWRUPD_FILE, "w");
    CU_ASSERT_NOT_EQUAL_FATAL(fd, NULL);
    CU_ASSERT_EQUAL_FATAL( fwrite(buff, sizeof(buff), 1, fd), 1 );
    fclose(fd);

    memset(&nodeP, 0, sizeof(nodeP));

    /* set severity for test*/
    g_fumo_work_thread_id = 0;
    nodeP.uri = testSeverity;
    nodeP.data_buffer = "3";
    nodeP.data_size = strlen(nodeP.data_buffer) + 1;
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);

    /** cancel for download */
    g_fumo_work_thread_id = 0;
    nodeP.uri = uriDownloadPkgUrl;
    nodeP.data_buffer = FUMO_DESCRIPTOR_URL;
    nodeP.data_size = strlen(FUMO_DESCRIPTOR_URL) + 1;

    //remove("./data/fumo.state.store");

    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(g_interface->execFunc(uriDownload, cmdData,
            correlator, context), FUMO_RC_ACCEPTED_FOR_PROCESSING);
    tid = g_fumo_work_thread_id;
    //sleep(1);
    CU_ASSERT_EQUAL(g_interface->execFunc(uriDownloadCancel, cmdData,
        correlator, context), MO_ERROR_NONE);
    CU_ASSERT_FALSE( pthread_join(tid, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    /** cancel for update */
    g_fumo_work_thread_id = 0;
    //remove("./data");
    CU_ASSERT_EQUAL( fumo_storage_set_download_descriptor(&dd),
            MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->execFunc(uriUpdate, cmdData, correlator,
                        context), FUMO_RC_ACCEPTED_FOR_PROCESSING);
    tid = g_fumo_work_thread_id;
    sleep(1);
    CU_ASSERT_EQUAL(g_interface->execFunc(uriUpdateCancel, cmdData,
        correlator, context), MO_ERROR_NONE);
    CU_ASSERT_FALSE( pthread_join(tid, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    /** cancel for download & update */
    g_fumo_work_thread_id = 0;
    nodeP.uri = uriDAUPkgUrl;
    nodeP.data_buffer = FUMO_DESCRIPTOR_URL;
    nodeP.data_size = strlen(FUMO_DESCRIPTOR_URL) + 1;

    //remove("./data");
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->execFunc(uriDAU, cmdData, correlator,
                        context), FUMO_RC_ACCEPTED_FOR_PROCESSING);
    tid = g_fumo_work_thread_id;
    sleep(1);
    CU_ASSERT_EQUAL(g_interface->execFunc(uriDAUCancel, cmdData,
        correlator, context), MO_ERROR_NONE);
    CU_ASSERT_FALSE( pthread_join(tid, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    g_interface->closeFunc(context);
}

void test_case_downloading_wifi_only_false()
{
    printf("\n\n>>>>>>>>>> test_case_downloading_wifi_only_false\n");
    char uriPkgURL[] = "./ManagedObjects/FUMO/Download/PkgURL";
    dmtree_node_t nodeP;
    void *context = NULL;
    void *threadRC = NULL;
    struct stat stInp, stOut;
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;
    pthread_t tid = 0;

    memset(&nodeP, 0, sizeof(nodeP));

    remove("./data/fumo.state.store");

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);

    CU_ASSERT_EQUAL(g_interface->initFunc(&context), MO_ERROR_NONE);

    CU_ASSERT_EQUAL(g_interface->isNodeFunc(uriPkgURL, &type, context),
                MO_ERROR_NONE);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    /* set severity for test*/
    g_fumo_work_thread_id = 0;
    nodeP.uri = FUMO_BASE_URI "/" FUMO_URI_EXT_SEVERITY;
    nodeP.data_buffer = "3";
    nodeP.data_size = strlen(nodeP.data_buffer) + 1;
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);

    /* set wifi_only false for test*/
    g_fumo_work_thread_id = 0;
    nodeP.uri = FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY;
    nodeP.data_buffer = "false";
    nodeP.data_size = strlen(nodeP.data_buffer) + 1;
    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);

    g_fumo_work_thread_id = 0;
    nodeP.uri = uriPkgURL;
    nodeP.data_buffer = FUMO_DESCRIPTOR_URL;
    nodeP.data_size = strlen(FUMO_DESCRIPTOR_URL) + 1;

    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);

    //remove("./data");
    int err = g_interface->execFunc("./ManagedObjects/FUMO/Download", "cmdData",
            "correlator", context);
    CU_ASSERT_EQUAL(err, FUMO_RC_ACCEPTED_FOR_PROCESSING);
    tid = g_fumo_work_thread_id;
    CU_ASSERT_FALSE( pthread_join(tid, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    CU_ASSERT_EQUAL( access("data/"DOWNLOAD_DESCRIPTOR_FILE_NAME, R_OK), 0 );
    CU_ASSERT_EQUAL( stat("download_descriptor_local.xml", &stInp), 0 );
    CU_ASSERT_EQUAL( stat("data/"DOWNLOAD_DESCRIPTOR_FILE_NAME, &stOut), 0 );
    CU_ASSERT_EQUAL( stInp.st_size, stOut.st_size );

    g_interface->closeFunc(context);
}

void test_case_restart_defer_timer_download()
{
    printf("\n\n>>>>>>>>>> test_case_restart_defer_timer_download\n");
    gui_supported = 1;

    char uriPkgURL[] = "./ManagedObjects/FUMO/Download/PkgURL";
    dmtree_node_t nodeP;
    void *context = NULL;
    void *threadRC = NULL;
    struct stat stInp, stOut;
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;
    pthread_t tid = 0;

    memset(&nodeP, 0, sizeof(nodeP));

    remove("./data/fumo.state.store");

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);

    CU_ASSERT_EQUAL(g_interface->initFunc(&context), MO_ERROR_NONE);

    CU_ASSERT_EQUAL(g_interface->isNodeFunc(uriPkgURL, &type, context),
            MO_ERROR_NONE);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    struct timespec ts_defer;
    clock_gettime(CLOCK_REALTIME, &ts_defer);
    ts_defer.tv_sec += 60;
    fumo_storage_set_long_value(offsetof(fumo_storage_t, defer_time), ts_defer.tv_sec);
    fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_DEFERRED);

    g_fumo_work_thread_id = 0;
    nodeP.uri = uriPkgURL;
    nodeP.data_buffer = FUMO_DESCRIPTOR_URL;
    nodeP.data_size = strlen(FUMO_DESCRIPTOR_URL) + 1;

    CU_ASSERT_EQUAL(g_interface->setFunc(&nodeP, context), MO_ERROR_NONE);

    //remove("./data");
    int err = g_interface->execFunc("./ManagedObjects/FUMO/Download", "cmdData",
            "correlator", context);
    CU_ASSERT_EQUAL(err, FUMO_RC_ACCEPTED_FOR_PROCESSING);
    tid = g_fumo_work_thread_id;
    CU_ASSERT_FALSE( pthread_join(tid, &threadRC) );
    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    CU_ASSERT_EQUAL( access("data/"DOWNLOAD_DESCRIPTOR_FILE_NAME, R_OK), 0 );
    CU_ASSERT_EQUAL( stat("download_descriptor_local.xml", &stInp), 0 );
    CU_ASSERT_EQUAL( stat("data/"DOWNLOAD_DESCRIPTOR_FILE_NAME, &stOut), 0 );
    CU_ASSERT_EQUAL( stInp.st_size, stOut.st_size );

    g_interface->closeFunc(context);
}

void all_tests(void)
{
    test_case_fumo_download();
    test_case_fumo_update();
    test_case_fumo_download_update();
    test_case_fumo_cancel();
    test_case_downloading_wifi_only_false();
    test_case_restart_defer_timer_download();
}

int main()
{
    CU_pSuite suite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    suite = CU_add_suite("fumo_functional_suite", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "all_tests",
            all_tests)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
/*
    if(0 == CU_add_test(suite, "fumo_download",
            test_case_fumo_download)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_update",
            test_case_fumo_update)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_download_update",
            test_case_fumo_download_update)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_cancel",
            test_case_fumo_cancel)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_downloading_wifi_only_false",
            test_case_downloading_wifi_only_false)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_restart_defer_timer_download",
            test_case_restart_defer_timer_download)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
*/
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

