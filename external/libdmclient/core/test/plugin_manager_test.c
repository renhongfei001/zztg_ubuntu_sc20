/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

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

void test_case_sample(void)
{
   CU_ASSERT(init_pluginManager());
}

int main(){
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if (NULL == CU_add_test(pSuite, "test semple", test_case_sample))
   {
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
