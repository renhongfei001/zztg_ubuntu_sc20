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
extern char* get_child_list_for_battery();
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
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface->closeFunc);
}

void test_is_node_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon";

    void * Data = NULL;
    omadmtree_node_kind_t node_type;
    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->isNodeFunc(node.uri, &node_type, Data), RESULT_SUCCESS);
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
    node.uri = "./ManagedObjects/DiagMon";

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
   node.uri = "./ManagedObjects/DiagMon/Network/Mode/PreferredNetworkMode";
   node.data_buffer = "01230123";
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
      CU_ASSERT_EQUAL(_iface->setFunc(&node, Data), MO_ERROR_COMMAND_FAILED);
   }
}

void test_set_no_data_func(void)
{
   dmtree_node_t node;
   memset(&node, 0, sizeof(dmtree_node_t));
   node.uri = "./ManagedObjects/DiagMon/Network/Mode/PreferredNetworkMode";
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
      CU_ASSERT_EQUAL(_iface->setFunc(&node, Data), MO_ERROR_INCOMPLETE_COMMAND);
   }
}

void test_set_success_func(void)
{
   dmtree_node_t node;
   memset(&node, 0, sizeof(dmtree_node_t));
   node.uri = "./ManagedObjects/DiagMon/Network/Mode/PreferredNetworkMode";
   node.data_buffer = "2";
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
      CU_ASSERT_EQUAL(_iface->setFunc(&node, Data), MO_ERROR_NONE);
   }
}

void test_set_not_found_func(void)
{
   dmtree_node_t node;
   memset(&node, 0, sizeof(dmtree_node_t));
   node.uri = "./ManagedObjects/Test";
   node.data_buffer = "2";
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
      CU_ASSERT_EQUAL(_iface->setFunc(&node, Data), MO_ERROR_NOT_FOUND);
   }
}

void test_set_not_allowed_func(void)
{
   dmtree_node_t node;
   memset(&node, 0, sizeof(dmtree_node_t));
   node.uri = "./ManagedObjects/DiagMon/Network/Mode";
   node.data_buffer = "2";
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
   char* uri = "./ManagedObjects/DiagMon/Network/GlobalDataRoamingAccess";

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->getACLFunc(uri, &value, Data), MO_ERROR_NONE);
   }
}

/** getACL for wrong URI test
 *
 * precondition: none
 * MO_ERROR_NOT_FOUND code should be returned
 **/
void test_get_acl_not(void)
{
   char *value;
   char* uri = "./ManagedObjects/DiagMon/Network/GlobalDataRoamingAccess1";

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->getACLFunc(uri, &value, Data), MO_ERROR_NOT_FOUND);
   }
}

/** findUrn test
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 **/
void test_find_urn(void)
{

   char *urn = "urn:oma:mo:oma-diag:1.0";

   char** uri = NULL;

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->findURNFunc(urn, &uri, Data), PLUGIN_SUCCESS);
   }
}

/** Test for executing node
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 **/
void test_exec_func(void)
{
   char* uri = "./ManagedObjects/DiagMon/WiFi/Operations/Disable";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), PLUGIN_SUCCESS);
   }
}

void test_exec_error_pal(void)
{
   char* uri = "./ManagedObjects/DiagMon/Network/Mode/Operations/GlobalMode";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_COMMAND_FAILED);
   }
}

void test_exec_not_found_func(void)
{
   char* uri = "./ManagedObjects/DiagMon/WiFi/Operations/1Disable";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NOT_FOUND);
   }
}

void test_not_allowed_exec_func(void)
{
   char* uri = "./ManagedObjects/DiagMon/WiFi/Enabled";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NOT_ALLOWED);
   }
}

void test_not_find_get_func(void)
{
   dmtree_node_t node;
   memset(&node, 0, sizeof(dmtree_node_t));
   node.uri = "./ManagedObjects/DiagMon/test";

   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NOT_FOUND);
   }
}

/** Test for getting leaf
 *
 * precondition: none
 * PLUGIN_SUCCESS code should be returned
 */
void test_get_int_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/WiFi/Speed";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), RESULT_SUCCESS);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "1");
    }
}

void test_get_float_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/GPS/SNR_InViewSatellites";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), RESULT_SUCCESS);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "0.5");
    }
}

void test_get_int_32_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Network/NetworkID";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), RESULT_SUCCESS);
    }
}

void test_get_char_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Network/NetworkCountryIso";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), RESULT_SUCCESS);
    }
}

void test_exec_enable_func(void)
{
   char* uri = "./ManagedObjects/DiagMon/Bluetooth/Discoverable/Operations/Enable";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NONE);
   }
}

void test_get_enable_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Bluetooth/Discoverable/Enabled";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "True");
    }
}

void test_exec_disable_func(void)
{
   char* uri = "./ManagedObjects/DiagMon/Bluetooth/Discoverable/Operations/Disable";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NONE);
   }
}

void test_get_disable_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Bluetooth/Discoverable/Enabled";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "False");
    }
}

void test_get_int_2_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Battery/2/Status";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}
void test_max_buf_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/WiFi/Networks";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_childs_storage(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Storage";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_get_x_childs_storage_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Storage/SDCard0";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_get_x_avail_storage_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Storage/SDCard0/Avail";
    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "1024.0");
    }
}

void test_get_x_used_storage_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Storage/SDCard0/Used/Pictures_Video";
    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_get_x_percentfree_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Storage/SDCard0/PercentFree";
    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "50");
    }
}

void test_childs_battery(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Battery";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "1/2");
    }
}

void test_get_x_childs_battery_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Battery/1";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_get_x_temp_battery_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Battery/1/Temp";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_get_x_level_battery_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Battery/1/Level";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_get_x_status_battery_func(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Battery/1/Status";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
    }
}

void test_get_x_childs_list(void)
{
    char *uri_bat = "./ManagedObjects/DiagMon/Battery";
    CU_ASSERT_STRING_EQUAL(get_child_list_for_battery(uri_bat), "1/2");
}

void test_bluetooth_childs_func(void)
{
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Bluetooth/Discoverable";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_STRING_EQUAL(get_child_list(Data, node.uri), "Enabled/Operations");
    }
}

void test_exec_mobile_data_enable(void)
{
   char* uri = "./ManagedObjects/DiagMon/Network/MobileData/Operations/Enable";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NONE);
   }
}

void test_get_mobile_data_enable(void)
{
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Network/MobileData/Enabled";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "True");
    }
}

void test_exec_mobile_data_disable(void)
{
   char* uri = "./ManagedObjects/DiagMon/Network/MobileData/Operations/Disable";
   const char *cmdData;
   const char *correlator;
   void * Data = NULL;
   CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
   int err = _iface->initFunc(&Data);
   if (err == PLUGIN_SUCCESS ){
       CU_ASSERT_EQUAL(_iface->execFunc(uri, cmdData, correlator, Data), MO_ERROR_NONE);
   }
}

void test_get_mobile_data(void)
{
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Network/MobileData/Enabled";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "False");
    }
}

void test_get_vlt_enable(void)
{
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/VOLTE/FeatureStatus/VLT/Setting/Enabled";

    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NONE);
        CU_ASSERT_STRING_EQUAL(node.data_buffer, "False");
    }
}

void test_get_nodes_storage(void){
    dmtree_node_t node;
    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = "./ManagedObjects/DiagMon/Storage/USB2.0";
    void * Data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(_iface);
    int err = _iface->initFunc(&Data);
    if (err == PLUGIN_SUCCESS ){
        CU_ASSERT_EQUAL(_iface->getFunc(&node, Data), MO_ERROR_NOT_FOUND);
    }
}

int main(){
    CU_pSuite suite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    suite = CU_add_suite("DiagmonCoreSuite", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test init func",
            test_init_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test close func",
            test_close_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }


    if(0 == CU_add_test(suite, "test is node func",
            test_is_node_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test get node func",
            test_get_node_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test set func",
            test_set_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test set no data func",
            test_set_no_data_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test set success func",
            test_set_success_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test set not found func",
            test_set_not_found_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test set not allowed func",
            test_set_not_allowed_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test get acl",
            test_get_acl)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test find urn",
            test_find_urn)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test exec func",
            test_exec_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test exec not found func",
            test_exec_not_found_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test not allowed exec func",
            test_not_allowed_exec_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }


    if(0 == CU_add_test(suite, "test_childs_storage",
            test_childs_storage)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_x_childs_storage_func",
            test_get_x_childs_storage_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "ctest_get_x_avail_storage_func",
            test_get_x_avail_storage_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "battery childs",
            test_childs_battery)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_x_childs_battery_func",
            test_get_x_childs_battery_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_x_temp_battery_func",
            test_get_x_temp_battery_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

   if(0 == CU_add_test(suite, "test_get_x_level_battery_func",
            test_get_x_level_battery_func)) {
        CU_cleanup_registry();
        return CU_get_error();
   }

   if(0 == CU_add_test(suite, "test_get_x_status_battery_func",
            test_get_x_status_battery_func)) {
        CU_cleanup_registry();
        return CU_get_error();
   }

    if(0 == CU_add_test(suite, "test not find get func",
            test_not_find_get_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test not find get func",
                        test_get_acl_not)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test get int func",
            test_get_int_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test get int 2 func",
            test_get_int_2_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test get float func",
            test_get_float_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test get int32 func",
            test_get_int_32_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test get char func",
            test_get_char_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "exec enable",
            test_exec_enable_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "get enable",
            test_get_enable_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "exec disable",
            test_exec_disable_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "get disable",
            test_get_disable_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "max buf for pal",
            test_max_buf_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_x_childs_list",
            test_get_x_childs_list)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_bluetooth_childs_func",
            test_bluetooth_childs_func)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_exec_mobile_data_enable",
            test_exec_mobile_data_enable)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_mobile_data_enable",
            test_get_mobile_data_enable)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_exec_mobile_data_disable",
            test_exec_mobile_data_disable)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_mobile_data",
            test_get_mobile_data)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_vlt_enable",
            test_get_vlt_enable)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_vlt_enable",
            test_get_x_percentfree_func)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_x_used_storage_func",
            test_get_x_used_storage_func)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_exec_error_pal",
            test_exec_error_pal)){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_get_nodes_storage",
            test_get_nodes_storage)){
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
