/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dm_logger.h"
#include "mo_omadmtree.h"
#include "mo_error.h"
#include "pal.h"
#include "plugin_utils.h"

#define PRV_BASE_URI "./ManagedObjects/ConnMO"
#define PRV_BASE_ACL "Get=*"

static void* palHandle = NULL;

static plugin_tree_node_t gNodes[] =
{

    {PRV_BASE_URI"", NULL,
        OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
        "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/ext",NULL,
        OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
        "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/ext/Settings", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/1", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/1/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/1/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class1_id_get", NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/1/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class1_name_get",
         "pal_network_apn_class1_name_set", NULL},
    {PRV_BASE_URI"/LTE/APN/1/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class1_ip_get",
         "pal_network_apn_class1_ip_set", NULL},
    {PRV_BASE_URI"/LTE/APN/1/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class1_state_get", NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/1/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/1/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class1_disabled"},
    {PRV_BASE_URI"/LTE/APN/2", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/2/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/2/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class2_id_get", NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/2/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class2_name_get",
         "pal_network_apn_class2_name_set", NULL},
    {PRV_BASE_URI"/LTE/APN/2/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class2_ip_get",
         "pal_network_apn_class2_ip_set", NULL},
    {PRV_BASE_URI"/LTE/APN/2/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class2_state_get", NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/2/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/2/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class2_disabled"},
    {PRV_BASE_URI"/LTE/APN/3", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*","",NULL,NULL,NULL},
    {PRV_BASE_URI"/LTE/APN/3/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*","",NULL,NULL,NULL},
    {PRV_BASE_URI"/LTE/APN/3/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class3_id_get", NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/3/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class3_name_get",
         "pal_network_apn_class3_name_set", NULL},
    {PRV_BASE_URI"/LTE/APN/3/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class3_ip_get",
         "pal_network_apn_class3_ip_set", NULL},
    {PRV_BASE_URI"/LTE/APN/3/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class3_state_get", NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/3/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/3/Setting/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class3_enable"},
    {PRV_BASE_URI"/LTE/APN/3/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class3_disabled"},
    {PRV_BASE_URI"/LTE/APN/4", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/4/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/4/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class4_id_get", NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/4/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class4_name_get",
         "pal_network_apn_class4_name_set", NULL},
    {PRV_BASE_URI"/LTE/APN/4/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class4_ip_get",
         "pal_network_apn_class4_ip_set", NULL},
    {PRV_BASE_URI"/LTE/APN/4/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class4_state_get", NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/4/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/LTE/APN/4/Setting/Operations/Enable", NULL ,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class4_enable"},
    {PRV_BASE_URI"/LTE/APN/4/Setting/Operations/Disable",NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class4_disabled"},
    {PRV_BASE_URI"/IMS", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/IMS/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/IMS/Setting/Domain", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_ims_domain_get", NULL, NULL},
    {PRV_BASE_URI"/IMS/Setting/sms_over_IP_network_indication", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_ims_sms_over_ip_network_indication_get",
         "pal_network_ims_sms_over_ip_network_indication_set", NULL},
    {PRV_BASE_URI"/IMS/Setting/smsformat", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_ims_smsformat_get",
         "pal_network_ims_smsformat_set", NULL},
    {PRV_BASE_URI"/eHRPD", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/1", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/1/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/1/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class1_id_get", NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/1/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class1_name_get",
         "pal_network_apn_class1_name_set", NULL},
    {PRV_BASE_URI"/eHRPD/APN/1/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class1_ip_get",
         "pal_network_apn_class1_ip_set", NULL},
    {PRV_BASE_URI"/eHRPD/APN/1/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class1_state_get", NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/1/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/1/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class1_disabled"},
    {PRV_BASE_URI"/eHRPD/APN/2", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/2/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/2/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class2_id_get", NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/2/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class2_name_get",
         "pal_network_apn_class2_name_set", NULL},
    {PRV_BASE_URI"/eHRPD/APN/2/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class2_ip_get",
         "pal_network_apn_class2_ip_set", NULL},
    {PRV_BASE_URI"/eHRPD/APN/2/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class2_state_get", NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/2/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/2/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class2_disabled"},
    {PRV_BASE_URI"/eHRPD/APN/3", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*","",NULL,NULL,NULL},
    {PRV_BASE_URI"/eHRPD/APN/3/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*","",NULL,NULL,NULL},
    {PRV_BASE_URI"/eHRPD/APN/3/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class3_id_get", NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/3/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class3_name_get",
         "pal_network_apn_class3_name_set", NULL},
    {PRV_BASE_URI"/eHRPD/APN/3/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class3_ip_get",
         "pal_network_apn_class3_ip_set", NULL},
    {PRV_BASE_URI"/eHRPD/APN/3/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class3_state_get", NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/3/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/3/Setting/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class3_enable"},
    {PRV_BASE_URI"/eHRPD/APN/3/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class3_disabled"},
    {PRV_BASE_URI"/eHRPD/APN/4", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/4/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/4/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class4_id_get", NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/4/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class4_name_get",
         "pal_network_apn_class4_name_set", NULL},
    {PRV_BASE_URI"/eHRPD/APN/4/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class4_ip_get",
         "pal_network_apn_class4_ip_set", NULL},
    {PRV_BASE_URI"/eHRPD/APN/4/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class4_state_get", NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/4/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/eHRPD/APN/4/Setting/Operations/Enable", NULL ,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class4_enable"},
    {PRV_BASE_URI"/eHRPD/APN/4/Setting/Operations/Disable",NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class4_disabled"},

    {PRV_BASE_URI"/3GPPLegacy", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/1", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/1/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/1/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class1_id_get", NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/1/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class1_name_get",
         "pal_network_apn_class1_name_set", NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/1/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class1_ip_get",
         "pal_network_apn_class1_ip_set", NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/1/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class1_state_get", NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/1/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/1/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class1_disabled"},
    {PRV_BASE_URI"/3GPPLegacy/APN/2", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/2/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/2/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class2_id_get", NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/2/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class2_name_get",
         "pal_network_apn_class2_name_set", NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/2/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class2_ip_get",
         "pal_network_apn_class2_ip_set", NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/2/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class2_state_get", NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/2/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/2/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class2_disabled"},
    {PRV_BASE_URI"/3GPPLegacy/APN/3", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*","",NULL,NULL,NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/3/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*","",NULL,NULL,NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/3/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class3_id_get", NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/3/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class3_name_get",
         "pal_network_apn_class3_name_set", NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/3/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class3_ip_get",
         "pal_network_apn_class3_ip_set", NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/3/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class3_state_get", NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/3/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/3/Setting/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class3_enable"},
    {PRV_BASE_URI"/3GPPLegacy/APN/3/Setting/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class3_disabled"},
    {PRV_BASE_URI"/3GPPLegacy/APN/4", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/4/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/4/Setting/Id", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class4_id_get", NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/4/Setting/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class4_name_get",
         "pal_network_apn_class4_name_set", NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/4/Setting/IP", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_apn_class4_ip_get",
         "pal_network_apn_class4_ip_set", NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/4/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_apn_class4_state_get", NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/4/Setting/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/3GPPLegacy/APN/4/Setting/Operations/Enable", NULL ,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class4_enable"},
    {PRV_BASE_URI"/3GPPLegacy/APN/4/Setting/Operations/Disable",NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_apn_class4_disabled"},

    {"NULL", NULL,NULL, OMADM_NOT_EXIST, NULL, NULL,NULL,NULL,NULL},
};

static int prv_init_fn(void **oData)
{
    palHandle = getPalHandle(PAL_INSTALL_DIR "/" PAL_LIB_NAME);

    if (!palHandle){
        DM_LOGE( "palHandle not initialised %s", dlerror());
        return MO_ERROR_COMMAND_FAILED;
        /**
         * \todo !!! function description and actual behaviour MUST be consistent.
         * according to "omadmtree_mo.h" this function shall return:
         * "SyncML error code"
         * but now we a returning something different.
         */
    }

    *oData = gNodes;
    return MO_ERROR_NONE;
}

static void prv_close_fn(void *iData)
{
    (void)iData;
	releasePalHandle(palHandle);
}

static int prv_get_fn(dmtree_node_t * nodeP,
                     void * iData)
{
    if (!palHandle){
        DM_LOGE( "ERROR! PAL isn't loaded");
        return MO_ERROR_COMMAND_FAILED; /// seems that we can't do anything if PAL not loaded
    }

    int i = 0;
    int err = MO_ERROR_COMMAND_FAILED;
    int value;
    int (*getLeafFunc)(data_buffer_t *);
    data_buffer_t *buffer = malloc(sizeof(data_buffer_t));

    if (buffer == NULL)
        return MO_ERROR_DEVICE_FULL;

    memset(buffer, 0, sizeof(data_buffer_t));
    buffer->size = MAX_BUFFER_SIZE;
    buffer->data = (char*) malloc(sizeof(char) * MAX_BUFFER_SIZE);
    if (buffer->data == NULL){
        if (buffer){
            free(buffer);
            buffer = NULL;
        }
        return MO_ERROR_DEVICE_FULL;
    }
    memset(buffer->data, 0, sizeof(char) * MAX_BUFFER_SIZE);
    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;

    if(nodeP) {
        i = prv_find_node(nodes, nodeP->uri);
        if (i == -1) {
            free_buffer(buffer);
            return MO_ERROR_NOT_FOUND;
        }
                if(palHandle && nodes[i].getLeafFuncName) {
                    getLeafFunc = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc) {
                        value = getLeafFunc(buffer);
                        if (value == MO_ERROR_NONE) {
                            free(nodeP->data_buffer);
                            nodeP->data_buffer = strdup(buffer->data);
                            if(nodeP->data_buffer) {
                                nodeP->data_size = strlen(buffer->data);
                                free(nodeP->format);
                                nodeP->format = strdup(nodes[i].format);
                                if(nodeP->format != NULL){
                                    err = MO_ERROR_NONE;
                                } else err = MO_ERROR_DEVICE_FULL;
                                if (nodeP->type) free(nodeP->type);
                                nodeP->type = strdup(nodes[i].type);
                                if(nodeP->type != NULL){
                                    err = MO_ERROR_NONE;
                                } else err = MO_ERROR_DEVICE_FULL;
                            }
                        } else {
                            switch(value){
                            case RESULT_MEMORY_ERROR:
                                err = MO_ERROR_DEVICE_FULL;
                                break;
                            case RESULT_ERROR_INVALID_STATE:
                                err = MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
                                break;
                            default:
                                err = MO_ERROR_COMMAND_FAILED;
                            }
                        }
                    } else {
                        err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    }
                } else {
                    char * child_list = NULL;
                    child_list = get_child_list(nodes, nodeP->uri);
                    if (child_list) {
                        nodeP->data_buffer = strdup(child_list);
                    } else {
                        nodeP->data_buffer = strdup(nodes[i].value);
                    }

                    if (nodeP->data_buffer) {
                        nodeP->data_size = strlen(nodeP->data_buffer);
                        free(nodeP->format);
                        nodeP->format = strdup(nodes[i].format);
                        if(nodeP->format){
                            err = MO_ERROR_NONE;
                        } else
                            err = MO_ERROR_DEVICE_FULL;
                        if (nodeP->type) free(nodeP->type);
                        nodeP->type = strdup(nodes[i].type);
                        if(nodeP->type != NULL){
                            err = MO_ERROR_NONE;
                        } else err = MO_ERROR_DEVICE_FULL;
                    }
                }

    }
    free_buffer(buffer);
    return err;
}

static int prv_set_fn(const dmtree_node_t * nodeP,
                     void * iData)
{
    if (!palHandle){
        DM_LOGD( "ERROR! PAL isn't loaded");
        return MO_ERROR_COMMAND_FAILED; /// seems that we can't do anything if PAL not loaded
    }

    int i = 0;
    int err = MO_ERROR_COMMAND_FAILED;
    int (*setLeafFunc)(data_buffer_t *);

    if(nodeP == NULL){
        return MO_ERROR_INCOMPLETE_COMMAND;
    }

    data_buffer_t *buffer = malloc(sizeof(data_buffer_t));

    if(buffer == NULL)
        return MO_ERROR_DEVICE_FULL;
    memset(buffer, 0, sizeof(data_buffer_t));
    buffer->data = strdup(nodeP->data_buffer);
    if (buffer->data){

        buffer->size = strlen(nodeP->data_buffer);
        plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;

        i = prv_find_node(nodes, nodeP->uri);
        if (i == -1) {
            free_buffer(buffer);
            return MO_ERROR_NOT_FOUND;
        }
        if(palHandle && nodes[i].setLeafFuncName) {
            setLeafFunc = dlsym(palHandle, nodes[i].setLeafFuncName);
            if (setLeafFunc) {
                err = setLeafFunc(buffer);
                if(err != MO_ERROR_NONE){
                    err = MO_ERROR_COMMAND_FAILED;
                }
            } else {
                err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
            }
        } else
            err = MO_ERROR_NOT_ALLOWED;

        free(buffer->data);
        buffer->data = NULL;
    }

    if(buffer){
        free(buffer);
        buffer = NULL;
    }
    return err;
}

static int prv_exec_fn (const char *iURI,
                       const char *cmdData,
                       const char *correlator,
                       void *iData)
{
    DM_LOGI("CONNMO prv_exec_fn {\n");
    if (!palHandle){
       DM_LOGD( "ERROR! PAL isn't loaded. \n");
        return MO_ERROR_COMMAND_FAILED; /// seems that we can't do anything if PAL not loaded
    }
    if(iData == NULL) {
        DM_LOGD(" ERROR: invalid paraneters\n");
        return MO_ERROR_DEVICE_FULL;
    }
    int i = 0;
    int err = MO_ERROR_COMMAND_FAILED;
    int (*execLeafFunc)(data_buffer_t *);
    data_buffer_t *buffer = malloc(sizeof(data_buffer_t));
    if (buffer == NULL )
        return MO_ERROR_DEVICE_FULL;

    buffer->size = MAX_BUFFER_SIZE;
    buffer->data = (char*) malloc(sizeof(char) * MAX_BUFFER_SIZE);
    if (buffer->data == NULL){
        if (buffer){
            free(buffer);
            buffer = NULL;
        }
        return MO_ERROR_DEVICE_FULL;
    }
    memset(buffer->data, 0, sizeof(char) * MAX_BUFFER_SIZE);
    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;

    if(iURI) {
        i = prv_find_node(nodes, iURI);
        if (i == -1) {
            return MO_ERROR_NOT_ALLOWED;
        }
                if(palHandle && nodes[i].execLeafFuncName) {
                    execLeafFunc = dlsym(palHandle, nodes[i].execLeafFuncName);
                    if (execLeafFunc) {
                        err = execLeafFunc(buffer);
                    } else {
                        err = MO_ERROR_NOT_ALLOWED;
                    }
                } else
                    err = MO_ERROR_NOT_ALLOWED;

    }
    if (PLUGIN_SUCCESS == err) {
        //TODO Do we need to reassign err?
    }
    free_buffer(buffer);
    return err;
}

omadm_mo_interface_t * omadm_get_mo_interface()
{
    omadm_mo_interface_t *retVal = NULL;

    retVal = malloc(sizeof(*retVal));
    if (retVal) {
        memset(retVal, 0, sizeof(*retVal));
        retVal->base_uri = strdup(PRV_BASE_URI);
        retVal->isNodeFunc = prv_mo_is_node;
        retVal->initFunc = prv_init_fn;
        retVal->closeFunc = prv_close_fn;
        retVal->findURNFunc = prv_find_urn;
        retVal->getFunc = prv_get_fn;
        retVal->setFunc = prv_set_fn;
        retVal->getACLFunc = prv_get_acl_fn;
        retVal->execFunc = prv_exec_fn;
    }

    return retVal;

}
