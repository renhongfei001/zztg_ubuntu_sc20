/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "fumo.h"

int init_suite()
{
    return 0;
};

int clean_suite()
{
    return 0;
};

void test_case_fumo_storage_open()
{
    CU_ASSERT_EQUAL( fumo_storage_open(), MO_ERROR_NONE);
    CU_ASSERT_EQUAL( fumo_storage_close(), MO_ERROR_NONE);
}

void test_case_pkg_name()
{
    //int fumo_storage_set_pkg_name(char *pgk_name)
    //int fumo_storage_get_pkg_name(char **pgk_name)
    char n[] = "int fumo_storage_set_pkg_name(char *pgk_name)";
    char *nn = NULL;

    CU_ASSERT_EQUAL( fumo_storage_set_pkg_name(NULL), MO_ERROR_INCOMPLETE_COMMAND );
    CU_ASSERT_EQUAL( fumo_storage_set_pkg_name(n), MO_ERROR_NONE );

    CU_ASSERT_EQUAL( fumo_storage_get_pkg_name(NULL), MO_ERROR_INCOMPLETE_COMMAND );
    CU_ASSERT_EQUAL( fumo_storage_get_pkg_name(&nn), MO_ERROR_NONE );
    CU_ASSERT_FALSE( strcmp(n, nn) );

    CU_ASSERT_EQUAL( fumo_storage_set_pkg_name(""), MO_ERROR_NONE );
    CU_ASSERT_EQUAL( fumo_storage_get_pkg_name(&nn), MO_ERROR_NOT_FOUND );
}


void test_case_pkg_version()
{
    //int fumo_storage_set_pkg_version(char *pgk_name)
    //int fumo_storage_get_pkg_version(char **pgk_name)
    char n[] = "int fumo_storage_set_pkg_version(char *pgk_name)";
    char *nn = NULL;

    CU_ASSERT_EQUAL( fumo_storage_set_pkg_version(NULL), MO_ERROR_INCOMPLETE_COMMAND );
    CU_ASSERT_EQUAL( fumo_storage_set_pkg_version(n), MO_ERROR_NONE );

    CU_ASSERT_EQUAL( fumo_storage_get_pkg_version(NULL), MO_ERROR_INCOMPLETE_COMMAND );
    CU_ASSERT_EQUAL( fumo_storage_get_pkg_version(&nn), MO_ERROR_NONE );
    CU_ASSERT_FALSE( strcmp(n, nn) );

    CU_ASSERT_EQUAL( fumo_storage_set_pkg_version(""), MO_ERROR_NONE );
    CU_ASSERT_EQUAL( fumo_storage_get_pkg_version(&nn),MO_ERROR_NOT_FOUND );
}


void test_fumo_storage_get_update_descriptor()
{
    //int fumo_storage_get_update_descriptor(pal_update_descriptor_t *update_descriptor)
    pal_download_descriptor_t dd;
    char upd_path[PATH_MAX]="";

    memset(&dd, 0, sizeof(dd));
    CU_ASSERT_EQUAL( fumo_storage_set_download_descriptor(&dd),
        MO_ERROR_NONE);

    pal_update_descriptor_t pud;
    CU_ASSERT_EQUAL(fumo_storage_get_update_descriptor(NULL),
                                                MO_ERROR_INCOMPLETE_COMMAND);
    memset(&pud, 0, sizeof(pud));
    CU_ASSERT_EQUAL(fumo_storage_get_update_descriptor(&pud),
                                                        MO_ERROR_NOT_FOUND);

    dd.packageName = "packageName";
    dd.ddVersion = "ddVersion";
    dd.objectUri = "objectUri";
    dd.size = 12345;
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

    CU_ASSERT_EQUAL( fumo_storage_set_download_descriptor(&dd),
                                                        MO_ERROR_NONE);

    memset(&pud, 0, sizeof(pud));
    CU_ASSERT_EQUAL(fumo_storage_get_update_descriptor(&pud),
                                                        MO_ERROR_NONE);

    snprintf(upd_path, PATH_MAX, "%s%s", MO_WORK_PATH"/data/", dd.packageName);
    CU_ASSERT_FALSE( strcmp( pud.name, upd_path ));
    CU_ASSERT_EQUAL( pud.size, dd.size );
    CU_ASSERT_FALSE( strcmp( pud.type, dd.type ));
    CU_ASSERT_FALSE( strcmp( pud.vendor, dd.vendor ));
    /** @todo make it right
    CU_ASSERT_FALSE( strcmp( pud.install_param, dd.installParam ));
    */
}

void test_fumo_storage_set_download_descriptor()
{
    //int fumo_storage_set_download_descriptor(
    //                         pal_download_descriptor_t *download_desciptor)

    pal_download_descriptor_t dd;
    memset(&dd, 0, sizeof(dd));

    dd.packageName = "packageName";
    dd.ddVersion = "ddVersion";
    dd.objectUri = "objectUri";
    dd.size = 12345;
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

    CU_ASSERT_EQUAL( fumo_storage_set_download_descriptor(&dd),
        MO_ERROR_NONE);
}


void test_fumo_storage_get_set_state()
{
    int state = -1;

    CU_ASSERT_EQUAL(fumo_storage_get_state(0), MO_ERROR_INCOMPLETE_COMMAND );

    CU_ASSERT_EQUAL(fumo_storage_get_state(&state), MO_ERROR_NONE);
    //CU_ASSERT_EQUAL(state, FUMO_DEVICE_STATE_IDLE);

    CU_ASSERT_EQUAL(fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_FAILED),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL(fumo_storage_get_state(&state), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(state, FUMO_DEVICE_STATE_DOWNLOAD_FAILED);

    CU_ASSERT_EQUAL(fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL(fumo_storage_get_state(&state), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(state, FUMO_DEVICE_STATE_IDLE);
}

void test_fumo_storage_get_set_string_value(void)
{
    //int fumo_storage_get_string_value
    //      (int offset, size_t max_len, char** value)
    //int fumo_storage_set_string_value
    //      (int offset, size_t max_len, char* value)

    char *pkgname = NULL;
    CU_ASSERT_EQUAL(fumo_storage_set_string_value(
        offsetof(fumo_storage_t, pkg_name), NAME_MAX,
        "pkg_name", strlen("pkg_name")), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(fumo_storage_get_string_value(
        offsetof(fumo_storage_t, pkg_name), NAME_MAX, &pkgname), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(strcmp("pkg_name", pkgname), 0);
}

void test_fumo_storage_get_set_int_value(void)
{
    //int fumo_storage_set_int_value(int offset, int value)
    //int fumo_storage_get_int_value(int offset, int *value)

    int v_inp = 123;
    int v_out = 0;

    CU_ASSERT_EQUAL(fumo_storage_set_int_value(
        offsetof(fumo_storage_t, state), v_inp), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(fumo_storage_get_int_value(
        offsetof(fumo_storage_t, state), &v_out), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(v_inp, v_out);
}

int main()
{
    CU_pSuite suite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    suite = CU_add_suite("fumo_state_storage_suite", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_storage_open",
            test_case_fumo_storage_open)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "pkg_name",
            test_case_pkg_name)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "pkg_version",
            test_case_pkg_version)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_storage_get_update_descriptor",
            test_fumo_storage_get_update_descriptor)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_storage_set_download_descriptor",
            test_fumo_storage_set_download_descriptor)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_storage_get_set_state",
                test_fumo_storage_get_set_state)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_storage_get_set_string_value",
            test_fumo_storage_get_set_string_value)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_storage_get_set_int_value",
            test_fumo_storage_get_set_int_value)) {
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
