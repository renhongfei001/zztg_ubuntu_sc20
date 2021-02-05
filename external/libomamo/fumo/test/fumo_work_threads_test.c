/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <unistd.h>

#include "fumo.h"

extern int
download_on_progess(void *context, unsigned int progress, int err_code);
extern int fumo_update_cancel(void);
extern int check_avail_space(unsigned long int);
pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
extern int gui_supported;

int init_suite()
{
    return 0;
};

int clean_suite()
{
    char* path = MO_WORK_PATH"/data/updateInfo.json";
    if ( access(path,F_OK) == 0) {
        if(remove(path)) {
            printf("\nFUMO TEST: Error removing file %s\n",path);
    } else
        printf("\nFUMO TEST: file %s removed\n",path);
    } else
        printf("\nFUMO TEST: file %s not exist\n",path);
    return 0;
};

extern int update_progress(void *context, unsigned int progress, int err_code);
void test_case_fumo_update_progress()
{
    //int update_progress(void *context, unsigned int progress, int err_code)

    CU_ASSERT_EQUAL(update_progress(
        NULL, 1, PAL_RC_FRMW_UPD_COMPLETED_SUCCESS), MO_ERROR_NONE);

    CU_ASSERT_EQUAL(update_progress(
        NULL, 1, PAL_RC_FRMW_UPD_CANCELLED), MO_ERROR_NONE);

    CU_ASSERT_EQUAL(update_progress(
        NULL, 1, PAL_RC_FRMW_UPD_INPROGRESS), MO_ERROR_NONE);

    CU_ASSERT_EQUAL(update_progress(
        NULL, 1, PAL_RC_FRMW_UPD_COMPLETED_FAILED), MO_ERROR_NONE);

    CU_ASSERT_EQUAL(update_progress(
        NULL, 1, 42), MO_ERROR_INVALID_CREDENTIALS);
}

extern void send_alarm_to_client(fumo_thread_args* fta, int alarm,
    const char* alert_type);

char cmd_data[] = "command data";
char correlator[] = "correlator";
int omadm_event_handler_called = 0;

int omadm_event_handler (omadm_mo_event_t *event)
{
    omadm_event_handler_called = 1;
    CU_ASSERT_EQUAL(event->correlator, correlator);
    CU_ASSERT_EQUAL(strcmp(event->dmclt_item.source, "node_uri"), 0);
    CU_ASSERT_EQUAL(strcmp(event->dmclt_item.target, "com.vzwdmserver"), 0);
    CU_ASSERT_EQUAL(strcmp(event->dmclt_item.type, "alert_type"), 0);
    CU_ASSERT_EQUAL(strcmp(event->dmclt_item.format, "chr"), 0);
    CU_ASSERT_EQUAL(strcmp(event->dmclt_item.data, "200"), 0);
    return 0;
}

void test_case_fumo_send_alarm_to_client()
{
    fumo_node node;
    fumo_thread_args fta = {
        fte_Download,
        &node,
        cmd_data,
        correlator
    };

    memset(&node, 0, sizeof(node));
    node.uri_ = "node_uri";
    node.format_ = efnf_Char;

    CU_ASSERT_EQUAL(
        omadm_mo_register_event_handler(NULL, omadm_event_handler ), 0 );

    send_alarm_to_client(&fta, 200, "alert_type");

    CU_ASSERT_EQUAL(omadm_event_handler_called, 1);

    CU_ASSERT_EQUAL(
        omadm_mo_unregister_event_handler(NULL, omadm_event_handler), 0);
}


void test_case_download_on_progess(void)
{
    // int download_on_progess(void *context, unsigned int progress, int err_code)
    /** @todo */
};


void test_case_update_cancel(void)
{
    // int fumo_update_cancel(void)
    /** @todo */
}

void test_case_show_check_memory_notification(void)
{
    CU_ASSERT_EQUAL(show_gui_dialog_check_memory(), MO_ERROR_NONE);
};

void test_case_show_low_battery_notification(void)
{
    CU_ASSERT_EQUAL(show_low_batt_notification(), MO_ERROR_NONE);
};

void test_case_show_download_fail(void)
{
    CU_ASSERT_EQUAL(show_gui_notification_download_fail() ,MO_ERROR_NONE);
};

void test_case_show_download_over_wifi_dialogs(void)
{
    CU_ASSERT_EQUAL(show_gui_system_update_wifi_not_connected() ,MO_ERROR_NONE);
};

void test_case_hide_download_over_wifi_dialogs(void)
{
    CU_ASSERT_EQUAL(hide_gui_system_update_wifi_not_connected() ,MO_ERROR_NONE);
};

void test_case_show_system_update_failed_dialog(void)
{
    CU_ASSERT_EQUAL(show_system_update_failed() ,MO_ERROR_NONE);
};

void fumo_test_case_size_is_bigger_than_2GB(void)
{
    unsigned long int size  =-1;
    CU_ASSERT_EQUAL(check_avail_space(size),
                    MO_ERROR_DEVICE_FULL);
}

void test_case_show_system_update_available_notification(void)
{
    CU_ASSERT_EQUAL(show_system_update_available_notification(
            "Message", "Url", 0) ,MO_ERROR_NONE);
    CU_ASSERT_EQUAL(show_system_update_available_notification(
            "Message", "Url", 1) ,MO_ERROR_NONE);
}

void test_case_show_downloading_system_update_notification(void)
{
    CU_ASSERT_EQUAL(show_downloading_system_update_notification(
            "Message") ,MO_ERROR_NONE);
}

void test_case_show_system_update_ready_to_install_notification(void)
{
    CU_ASSERT_EQUAL(show_system_update_ready_to_install_notification()
            ,MO_ERROR_NONE);
}

void test_case_show_installing_system_update_notification(void)
{
    CU_ASSERT_EQUAL(show_installing_system_update_notification()
            ,MO_ERROR_NONE);
}

void test_case_show_system_update_ready_restart_notification(void)
{
    CU_ASSERT_EQUAL(show_system_update_ready_restart_notification()
            ,MO_ERROR_NONE);
}

void test_case_show_update_error_dialog(void)
{
    char *make = "Make";
    char *model = "Model";
    show_update_error_dialog(make, model);
    CU_PASS("show_update_error_dialog(): There were not segfaults");
}

void test_case_show_update_complete_dialog(void)
{
    char *make = "Make";
    char *model = "Model";
    show_update_complete_dialog(make, model);
    CU_PASS("show_update_complete_dialog(): There were not segfaults");
}

extern pthread_mutex_t gui_wait_mutex;
extern pthread_cond_t gui_wait_conditional;
extern pal_fumo_gui_user_reply_param gui_response;

void* user_reaction(void* args)
{
    int delay = *((int*)args);
    sleep(delay);
    gui_response.button_id = 123;
    pthread_mutex_lock(&gui_wait_mutex);
    pthread_cond_signal(&gui_wait_conditional);
    pthread_mutex_unlock(&gui_wait_mutex);
    return NULL;
}

void test_case_wait_for_user_reaction(void)
{
    pthread_mutex_lock(&test_mutex);
    sleep(1);
    int delay = 1;
    gui_response.button_id = 0;
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, user_reaction, &delay);
    int button = wait_for_user_reaction();
    CU_ASSERT_EQUAL(button, 123);
    pthread_mutex_unlock(&test_mutex);
}

void test_case_wait_for_user_reaction_or_timer(void)
{
    pthread_mutex_lock(&test_mutex);
    sleep(1);
    int delay = 1;
    gui_response.button_id = 0;
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, user_reaction, &delay);
    int wait_result = wait_for_user_reaction_or_timer(5);
    CU_ASSERT_EQUAL(wait_result, 123);
    delay = 5;
    pthread_create(&thread_id, NULL, user_reaction, &delay);
    wait_result = wait_for_user_reaction_or_timer(1);
    CU_ASSERT_EQUAL(wait_result, 0);
    pthread_mutex_unlock(&test_mutex);
}

extern pthread_mutex_t wifi_wait_mutex;
extern pthread_cond_t wifi_wait_conditional;

void* wifi_status_listener(void* args)
{
    int delay = *((int*)args);
    sleep(delay);
    pthread_mutex_lock(&wifi_wait_mutex);
    pthread_cond_signal(&wifi_wait_conditional);
    pthread_mutex_unlock(&wifi_wait_mutex);
    return NULL;
}

void test_case_wait_wifi_timer(void)
{
    pthread_mutex_lock(&test_mutex);
    sleep(1);
    int delay = 1;
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, wifi_status_listener, &delay);
    int wait_result = wait_wifi();
    CU_ASSERT_EQUAL(wait_result, 0);
    pthread_mutex_unlock(&test_mutex);
}

extern pthread_mutex_t defer_wait_mutex;
extern pthread_cond_t defer_wait_conditional;

void* defer_status_listener(void* args)
{
    int delay = *((int*)args);
    sleep(delay);
    pthread_mutex_lock(&defer_wait_mutex);
    pthread_cond_signal(&defer_wait_conditional);
    pthread_mutex_unlock(&defer_wait_mutex);
    return NULL;
}

void test_case_wait_defer_timer(void)
{
    pthread_mutex_lock(&test_mutex);
    sleep(1);
    int delay = 1;
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, defer_status_listener, &delay);
    long defer_time = time(NULL) + 10;
    int wait_result = wait_defer_timer(defer_time);
    CU_ASSERT_EQUAL(wait_result, 0);
    pthread_mutex_unlock(&test_mutex);
}

int main()
{
    CU_pSuite suite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    suite = CU_add_suite("fumo_hreads_suite", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_update_progress",
            test_case_fumo_update_progress)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_send_alarm_to_client",
            test_case_fumo_send_alarm_to_client)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_download_on_progess",
            test_case_download_on_progess)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_update_cancel",
            test_case_update_cancel)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_check_memory_notification",
            test_case_show_check_memory_notification)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_low_battery_notification",
        test_case_show_low_battery_notification)) {

        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_download_fail",
            test_case_show_download_fail)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_test_case_size_is_bigger_than_2GB",
            fumo_test_case_size_is_bigger_than_2GB)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_system_update_available_notification",
            test_case_show_system_update_available_notification)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_downloading_system_update_notification",
            test_case_show_downloading_system_update_notification)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_system_update_ready_to_install_notification",
            test_case_show_system_update_ready_to_install_notification)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_download_over_wifi_dialogs",
            test_case_show_download_over_wifi_dialogs)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_hide_download_over_wifi_dialogs",
            test_case_hide_download_over_wifi_dialogs)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (0 == CU_add_test(suite, "test_case_show_system_update_failed_dialog",
            test_case_show_system_update_failed_dialog)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_installing_system_update_notification",
            test_case_show_installing_system_update_notification)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_system_update_ready_restart_notification",
            test_case_show_system_update_ready_restart_notification)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_update_error_dialog",
            test_case_show_update_error_dialog)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_show_update_complete_dialog",
            test_case_show_update_complete_dialog)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_wait_for_user_reaction",
            test_case_wait_for_user_reaction)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_wait_for_user_reaction_or_timer",
            test_case_wait_for_user_reaction_or_timer)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_wait_wifi_timer",
            test_case_wait_wifi_timer)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_wait_defer_timer",
            test_case_wait_defer_timer)) {
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

