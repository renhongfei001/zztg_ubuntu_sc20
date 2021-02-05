/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h> //alarm()

extern void run_main_loop();
extern void handle_signal(int sig);
extern void stop_main_loop();
extern bool test_main_loop_running();
extern int init(int argc, char**argv);
extern int omadm_event_handler_init();

int init_suite(void)
{
    return 0;
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite(void)
{
    return 0;
}

void handle_alarm_stop (int sig)
{
    (void)sig;
    CU_ASSERT(test_main_loop_running());
    stop_main_loop();
    CU_ASSERT(!test_main_loop_running());
}

void handle_alarm_default (int sig)
{
    (void)sig;
    CU_ASSERT(test_main_loop_running());
    handle_signal(SIGINT);
    CU_ASSERT(!test_main_loop_running());
}

void test_main_loop(void)
{
    signal(SIGALRM, handle_alarm_stop);
    alarm(2);
    run_main_loop();
}

void test_init(void)
{
    signal(SIGALRM, handle_alarm_default);
    alarm(2);
    char *ar0 = "./init-TEST-vzwcore";
    char *arr[1];
    arr[0] = ar0;
    int res = init(1, arr);
    CU_ASSERT_EQUAL(res, 0);
}

int main(){
    CU_TestInfo testsCore[] = {
        {"Main loop test", test_main_loop},
        {"Init routine test", test_init},
        CU_TEST_INFO_NULL,
    };

    CU_SuiteInfo suites[] = {
        {"DMC Init Suite", init_suite, clean_suite, testsCore},
        CU_SUITE_INFO_NULL,
    };

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    if( CU_register_suites( suites ) != CUE_SUCCESS ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_SILENT);
    CU_basic_run_tests();
    unsigned int failedTests = CU_get_number_of_tests_failed();
    if(failedTests > 0)
    {
        printf("Failure list:");
        CU_basic_show_failures(CU_get_failure_list());
        printf("\n");
    }
    CU_ErrorCode errorResult = CU_get_error();
    CU_cleanup_registry();
    return errorResult == CUE_SUCCESS ? failedTests : errorResult;
}
