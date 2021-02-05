/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dm_logger.h"
#include "mo_omadmtree.h"
#include "mo_error.h"
#include "pal.h"
#include "plugin_utils.h"

#define PRV_BASE_URI "./ManagedObjects/DiagMon"
#define PRV_BASE_ACL "Get=*"
#define PRV_URN      "urn:oma:mo:oma-diag:1.0"
#define PRV_URN_BATTERY "urn:oma:mo:omadiag:batteryinfo:1.0"
#define PRV_URN_MEMORY "urn:oma:mo:omadiag:memory:1.0"
#define WIFI_NETWORKS_URI "./ManagedObjects/DiagMon/WiFi/Networks"

#define DENY_ROAMING "./ManagedObjects/DiagMon/Network/Operations/DenyRoaming"
#define ALLOW_ROAMING "./ManagedObjects/DiagMon/Network/Operations/AllowCurrentTripRoaming"
#define MOBILE_DATA_ENABLE "./ManagedObjects/DiagMon/Network/MobileData/Operations/Enable"
#define MOBILE_DATA_DISABLE "./ManagedObjects/DiagMon/Network/MobileData/Operations/Disable"

#define STORAGE_URI "./ManagedObjects/DiagMon/Storage"
#define BATTERY_URI "./ManagedObjects/DiagMon/Battery"

#define STORAGE_URI_SL "./ManagedObjects/DiagMon/Storage/"
#define BATTERY_URI_SL "./ManagedObjects/DiagMon/Battery/"

#define WITHOUT_X 0
#define X_BATTERY 1
#define X_STORAGE 2

static void* palHandle = NULL;

static plugin_tree_node_t gNodes[] =
{
    {PRV_BASE_URI, PRV_URN,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/RF", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/RF/CurrentSystem", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/RF/CurrentSystem/Voice", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_curr_voice_get", NULL, NULL},
    {PRV_BASE_URI"/RF/CurrentSystem/Data", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_curr_data_get", NULL, NULL},
    {PRV_BASE_URI"/RF/HomeRoam", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_homeroam_get", NULL, NULL},
    {PRV_BASE_URI"/Network", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Network/1xSignal", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_1xsignal_get", NULL, NULL},
    {PRV_BASE_URI"/Network/4GSignal", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_4gsignal_get", NULL, NULL},
    {PRV_BASE_URI"/Network/BaseStationId", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_basestationid_get", NULL, NULL},
    {PRV_BASE_URI"/Network/SystemID", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_systemid_get", NULL, NULL},
    {PRV_BASE_URI"/Network/MCC", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_mcc_get", NULL, NULL},
    {PRV_BASE_URI"/Network/NetworkID", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_networkid_get", NULL, NULL},
    {PRV_BASE_URI"/Network/Mode", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Network/Mode/SupportedModes", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_supported_modes_get", NULL, NULL},
    {PRV_BASE_URI"/Network/Mode/PreferredNetworkMode", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_preferred_network_mode_get",
         "pal_network_preferred_network_mode_set", NULL},
    {PRV_BASE_URI"/Network/Mode/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Network/Mode/Operations/GlobalMode", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_globalmode_enable"},
    {PRV_BASE_URI"/Network/Mode/Operations/LTE_CDMA_Mode", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_lte_cdma_mode_enable"},
    {PRV_BASE_URI"/Network/Mode/Operations/GSM_UMTS_Mode", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_network_gsm_umts_mode_enable"},
    {PRV_BASE_URI"/Network/GlobalDataRoamingAccess", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_network_global_data_roaming_access_get",
         "pal_network_global_data_roaming_access_set", NULL},
    {PRV_BASE_URI"/Network/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Network/Operations/DenyRoaming", NULL,
          OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
          "Exec=*", "", NULL, NULL, "pal_network_global_data_roaming_access_set"},
    {PRV_BASE_URI"/Network/Operations/AllowCurrentTripRoaming", NULL,
          OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
          "Exec=*", "", NULL, NULL, "pal_network_global_data_roaming_access_set"},
    {PRV_BASE_URI"/Network/MNC", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_mnc_get", NULL, NULL},
    {PRV_BASE_URI"/Network/3GSignal", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_3gsignal_get", NULL, NULL},
    {PRV_BASE_URI"/Network/CurrentNetwork", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_currentnetwork_get", NULL, NULL},
    {PRV_BASE_URI"/Network/CallState", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_callstate_get", NULL, NULL},
    {PRV_BASE_URI"/Network/ConnectType", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_connecttype_get", NULL, NULL},
    {PRV_BASE_URI"/Network/NetworkCountryIso", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_network_countryiso_get", NULL, NULL},
    {PRV_BASE_URI"/Network/SIMState", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_simstate_get", NULL, NULL},
    {PRV_BASE_URI"/Network/MobileData", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Network/MobileData/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_mobiledata_state_get",
         NULL, NULL},
    {PRV_BASE_URI"/Network/MobileData/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Network/MobileData/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL, "pal_network_mobiledata_state_set"},
    {PRV_BASE_URI"/Network/MobileData/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL, "pal_network_mobiledata_state_set"},
    {PRV_BASE_URI"/Battery", PRV_URN_BATTERY,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},

    {PRV_BASE_URI"/Battery/1", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Battery/1/Temp", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_temp_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/1/Level", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_level_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/1/Status", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_status_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/1/Type", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_type_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/1/Voltage", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_voltage_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/1/Plug_Status", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_plug_status_get", NULL, NULL},

    {PRV_BASE_URI"/Battery/2", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Battery/2/Temp", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_temp_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/2/Level", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_level_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/2/Status", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_status_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/2/Type", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_type_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/2/Voltage", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_voltage_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/2/Plug_Status", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_plug_status_get", NULL, NULL},

    {PRV_BASE_URI"/Battery/3", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Battery/3/Temp", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_temp_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/3/Level", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_level_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/3/Status", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_status_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/3/Type", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_type_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/3/Voltage", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_voltage_get", NULL, NULL},
    {PRV_BASE_URI"/Battery/3/Plug_Status", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_battery_x_plug_status_get", NULL, NULL},

    {PRV_BASE_URI"/Memory", PRV_URN_MEMORY,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Memory/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_memory_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Memory/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_memory_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Memory/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_memory_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", "pal_system_storages_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Used", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Used/Applications", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_used_applications_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Used/Pictures_Video", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_used_pictures_video_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Used/Audio", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_used_audio_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Used/Downloads", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_used_downloads_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Used/Miscellaneaous", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_used_miscellaneaous_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard0/Used/Cache", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_used_caches_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/SDCard1", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard1/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard1/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard1/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/SDCard2", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard2/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard2/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/SDCard2/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/USB3.0", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/USB3.0/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/USB3.0/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/USB3.0/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/USB2.0", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/USB2.0/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/USB2.0/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/USB2.0/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other1", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other1/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other1/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other1/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other2", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other2/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other2/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other2/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other3", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other3/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other3/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other3/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other4", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other4/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other4/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other4/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other5", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other5/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other5/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other5/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other6", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other6/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other6/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other6/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other7", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other7/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other7/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other7/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other8", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other8/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other8/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other8/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/Storage/Other9", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Storage/Other9/Avail", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_avail_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other9/PercentFree", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_system_storage_percentfree_get", NULL, NULL},
    {PRV_BASE_URI"/Storage/Other9/Total", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_storage_x_total_get", NULL, NULL},

    {PRV_BASE_URI"/WiFi", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/WiFi/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_wifi_state_get", NULL, NULL},
    {PRV_BASE_URI"/WiFi/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/WiFi/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_wifi_disable"},
    {PRV_BASE_URI"/WiFi/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_wifi_enable"},
    {PRV_BASE_URI"/WiFi/Speed", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_wifi_speed_get", NULL, NULL},
    {PRV_BASE_URI"/WiFi/Status", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_wifi_status_get", NULL, NULL},
    {PRV_BASE_URI"/WiFi/Networks", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_wifi_networks_get", NULL, NULL},
    {PRV_BASE_URI"/WiFi/Signal", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_wifi_signal_get", NULL, NULL},
    {PRV_BASE_URI"/WiFi/SSID", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_wifi_ssid_get", NULL, NULL},
    {PRV_BASE_URI"/WiFi/HotSpot", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/WiFi/HotSpot/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_wifi_hotspot_state_get", NULL, NULL},
    {PRV_BASE_URI"/WiFi/HotSpot/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/WiFi/HotSpot/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_wifi_hotspot_disable"},
    {PRV_BASE_URI"/WiFi/MAC", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_system_wifi_mac_get", NULL, NULL},
    {PRV_BASE_URI"/WiFi/BSSID", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_network_wifi_bssid_get", NULL, NULL},
    {PRV_BASE_URI"/Bluetooth", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/Name", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_bluetooth_name_get", NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_bluetooth_state_get", NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL, "pal_device_bluetooth_enable"},
    {PRV_BASE_URI"/Bluetooth/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL, "pal_device_bluetooth_disable"},
    {PRV_BASE_URI"/Bluetooth/HDP", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_bluetooth_hdp_get", NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/A2DPState", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_bluetooth_a2dpstate_get", NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/HSPState", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_bluetooth_hspstate_get", NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/Discoverable", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/Discoverable/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_bluetooth_discoverable_state_get", NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/Discoverable/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Bluetooth/Discoverable/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_bluetooth_discoverable_enable"},
    {PRV_BASE_URI"/Bluetooth/Discoverable/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_bluetooth_discoverable_disable"},
    {PRV_BASE_URI"/GPS", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/GPS/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_gps_state_get", NULL, NULL},
    {PRV_BASE_URI"/GPS/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/GPS/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_gps_enable"},
    {PRV_BASE_URI"/GPS/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_gps_disable"},
    {PRV_BASE_URI"/GPS/Num_of_Satellites", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_gps_num_of_satellites_get", NULL, NULL},
    {PRV_BASE_URI"/GPS/SNR_InViewSatellites", NULL,
         OMADM_LEAF_FORMAT_FLOAT, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_gps_snr_inviewsatellites_get", NULL, NULL},
    {PRV_BASE_URI"/NFC", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/NFC/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_nfc_state_get", NULL, NULL},
    {PRV_BASE_URI"/NFC/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/NFC/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_nfc_enable"},
    {PRV_BASE_URI"/NFC/Operations/Disable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_nfc_disable"},
    {PRV_BASE_URI"/Security", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Security/Encryption", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Security/Encryption/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_encryption_state_get", NULL, NULL},
    {PRV_BASE_URI"/Security/VerifyApp", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Security/VerifyApp/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "",
         "pal_device_verifyapp_state_get", NULL, NULL},
    {PRV_BASE_URI"/Security/VerifyApp/Operations", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Security/VerifyApp/Operations/Enable", NULL,
         OMADM_LEAF_FORMAT_CHAR, OMADM_LEAF_TYPE,
         "Exec=*", "", NULL, NULL,
         "pal_device_verifyapp_enable"},
    {PRV_BASE_URI"/Sound", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Sound/Volumes", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},
    {PRV_BASE_URI"/Sound/Volumes/Ringtone", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_device_volumes_ringtone_get",
         "pal_device_volumes_ringtone_set", NULL},
    {PRV_BASE_URI"/Sound/Volumes/Notifications", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_device_volumes_notifications_get",
         "pal_device_volumes_notifications_set", NULL},
    {PRV_BASE_URI"/Sound/Volumes/Alarms", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_diagmon_sound_volumes_alarms_get",
         "pal_device_volumes_alarms_set", NULL},
    {PRV_BASE_URI"/Sound/Volumes/Audio_Video_Media", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_device_volumes_media_get",
         "pal_device_volumes_media_set", NULL},
    {PRV_BASE_URI"/Sound/Volumes/Bluetooth", NULL,
         OMADM_LEAF_FORMAT_INT, OMADM_LEAF_TYPE,
         "Get=*&Replace=*", "",
         "pal_device_volumes_bluetooth_get",
         "pal_diagmon_sound_volumes_bluetooth_set", NULL},
    {PRV_BASE_URI"/VOLTE", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},                                 // The same fuction is definded in DCMO
    {PRV_BASE_URI"/VOLTE/FeatureStatus", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},                                 // The same fuction is definded in DCMO
    {PRV_BASE_URI"/VOLTE/FeatureStatus/VLT", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},                                 // The same fuction is definded in DCMO
    {PRV_BASE_URI"/VOLTE/FeatureStatus/VLT/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},                                 // The same fuction is definded in DCMO
    {PRV_BASE_URI"/VOLTE/FeatureStatus/VLT/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_mobile_vlt_state_get", NULL, NULL},                                 // The same fuction is definded in DCMO
    {PRV_BASE_URI"/VOLTE/FeatureStatus/LVC", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},                                 // The same fuction is definded in DCMO
    {PRV_BASE_URI"/VOLTE/FeatureStatus/LVC/Setting", NULL,
         OMADM_NODE_FORMAT, OMADM_NODE_TYPE,
         "Get=*", "", NULL, NULL, NULL},                                 // The same fuction is definded in DCMO
    {PRV_BASE_URI"/VOLTE/FeatureStatus/LVC/Setting/Enabled", NULL,
         OMADM_LEAF_FORMAT_BOOL, OMADM_LEAF_TYPE,
         "Get=*", "", "pal_mobile_lvc_state_get", NULL, NULL},                                 // The same fuction is definded in DCMO

     {"NULL", NULL, NULL, OMADM_NOT_EXIST, NULL, NULL, NULL, NULL, NULL},
};

//Frees buffer for *x
static void free_x(char *x)
{
    if (x){
        free(x);
        x = NULL;
    }
}

char* get_child_list_for_battery()
{
    int (*getLeafFunc_int32)(int32_t *);
    int32_t int32_buffer = 0;
    int err = MO_ERROR_NONE;
    int value = MO_ERROR_NONE;
    char data[10];
    char *childs;
    int length = 0;
    int k = 0;

    getLeafFunc_int32 = dlsym(palHandle, "pal_system_batteries_get");
    if (getLeafFunc_int32){
        value = getLeafFunc_int32(&int32_buffer);
    }
    else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;

    if (value != MO_ERROR_NONE) return NULL;

    if (int32_buffer == 0) {
        DM_LOGI("<X>: count of battaries = 0");
        err = MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
        return NULL;
    }
    if (int32_buffer == 1){
        childs = calloc(2, sizeof(char));
        if (childs){
            sprintf(childs, "%d", int32_buffer);
        } else err = MO_ERROR_DEVICE_FULL;
    } else {
        length = int32_buffer*2 -1;
        childs = (char*) calloc(length+1, sizeof(char));
        if (childs){
            for (k = 1; k < int32_buffer; k++){
                sprintf(data, "%d", k);
                strcat(childs,data);
                strcat(childs,"/");
            }
            sprintf(data, "%d", k);
            strcat(childs,data);
            DM_LOGI("<X>: child list = %s", childs);
        } else err = MO_ERROR_DEVICE_FULL;
    }

    if ( err == MO_ERROR_NONE){
        DM_LOGI("<X>: child list = %s", childs);
        return childs;
    } else {
        return NULL;
    }
}

/**
 * Determines the presence of the requested storage on the device
 *
 * @param[out] storage Name of the requested storage
 * @param[out] buffer Buffer for working with PAL
 * @param[out] present Flag of the existence of the requested object
 * @return @see enum mo_errors
 */
int storage_in_list(char *storage, data_buffer_t *buffer, int *present)
{
    int value = RESULT_ERROR;
    int (*getLeafFunc)(data_buffer_t *);

    getLeafFunc = dlsym(palHandle, "pal_system_storages_get");
    if (getLeafFunc) {
        value = getLeafFunc(buffer);
    } else return MO_ERROR_COMMAND_NOT_IMPLEMENTED;

    while(value == RESULT_BUFFER_OVERFLOW){
        char* old_data = buffer->data;
        buffer->data = (char*)realloc(buffer->data, buffer->size);
        if (buffer->data == NULL){
            value = RESULT_MEMORY_ERROR;
            free(old_data);
            break;
        }
        memset(buffer->data, 0, sizeof(char) * buffer->size);
        value = getLeafFunc(buffer);
    }

    if (value == RESULT_SUCCESS){
        if(strstr(buffer->data,storage)){
            *present = 1;
        } else {
            *present = 0;
        }
    } else return MO_ERROR_COMMAND_FAILED;
    return MO_ERROR_NONE;
}

/**
 * Initializes the object with plugin_tree_node_t struct
 *
 * @param[out] oData Contains plugin_tree_node_t data after initialization
 * @return PLUGIN_SUCCESS
 */
static int prv_init_fn(void **oData)
{
    palHandle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);

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

    if (palHandle) {
        // Not sure if we can do anything with error code provided by dlclose(),
        // so just print error and exit.
        int res = dlclose(palHandle);
        palHandle = NULL;
        if (0 != res)
           DM_LOGE("%s: palHandle not closed %s", __FILE__, dlerror());
    }
}

static int prv_get_fn(dmtree_node_t * nodeP,
                     void * iData)
{
	DM_LOGI("diagmo prv_get_fn was used\n");
    if (!palHandle){
        DM_LOGE( "ERROR! PAL isn't loaded");
        return MO_ERROR_COMMAND_FAILED; /// seems that we can't do anything if PAL not loaded
    }

    int i = 0;
    int err = MO_ERROR_NONE;
    int err_1 = MO_ERROR_NONE;
    int value = RESULT_ERROR;
    unsigned int pal_buffer = MAX_BUFFER_SIZE;
    int x = 0; // parameter for battery
    char *x_param = NULL; //parameter for storage
    int parameter = 0; // 0 - without x, 1 - battery, 2 - storage
    int item_is_present = 1;

    data_buffer_t *buffer;
    int32_t int32_buffer = 0;
    float float_buffer = 0;

    int (*getLeafFunc)(data_buffer_t *);
    int (*getLeafFunc_float)(float *);
    int (*getLeafFunc_int32)(int32_t *);

    int (*getLeafFunc_x)(int, data_buffer_t *);
    int (*getLeafFunc_float_x)(int, float *);
    int (*getLeafFunc_int32_x)(int, int32_t *);

    int (*getLeafFunc_float_storage)(const char *, float *);
    int (*getLeafFunc_int_storage)(const char *, int *);

    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;

    if (!nodeP) return MO_ERROR_COMMAND_FAILED;

    i = prv_find_node(nodes, nodeP->uri);
    if (i == -1) {
        return MO_ERROR_NOT_FOUND;
    } else {
        free(nodeP->format);
        nodeP->format = strdup(nodes[i].format);
        if(!nodeP->format){
            return MO_ERROR_DEVICE_FULL;
        }
    }

    buffer = (data_buffer_t *)malloc(sizeof(data_buffer_t));
    if (buffer == NULL)
        return MO_ERROR_DEVICE_FULL;

    memset(buffer, 0, sizeof(data_buffer_t));

    if(!strcmp(WIFI_NETWORKS_URI, nodeP->uri)){
        pal_buffer = MAX_BUFFER_SIZE_2000;
    }

    buffer->size = pal_buffer;
    buffer->data = (char*) calloc(pal_buffer, sizeof(char));
    if (buffer->data == NULL){
        free_buffer(buffer);
        free_x(x_param);
        return MO_ERROR_DEVICE_FULL;
    }

    DM_LOGI("URI:%s", nodeP->uri);
    if (strstr(nodeP->uri, BATTERY_URI_SL)){
        err_1 = parsing_x(nodeP->uri, &x_param);
        if (err_1 == MO_ERROR_NONE){
            x = atoi(x_param);
            parameter = 1;
            DM_LOGI("<X>: battery_x = %d", x);
        }
    } else if  (strstr(nodeP->uri, STORAGE_URI_SL)){
        DM_LOGI("<X>: start parsing storage");
        err_1 = parsing_x(nodeP->uri, &x_param);
        if (err_1 == MO_ERROR_NONE){
            parameter = 2;
            err_1 = storage_in_list(x_param, buffer, &item_is_present);
        }
    }

    if (err_1 != MO_ERROR_NONE){
        free_buffer(buffer);
        free_x(x_param);
        DM_LOGI("DiagMon_parsing_x_failed");
        return err_1;
    }

    if (item_is_present != 1){
        free_buffer(buffer);
        free_x(x_param);
        DM_LOGI("DiagMon_storage is not exist");
        return MO_ERROR_NOT_FOUND;
    }

    if( !(palHandle && nodes[i].getLeafFuncName)) {
        char * child_list = NULL;
        err_1 = MO_ERROR_NONE;
        if (!strcmp(nodeP->uri, BATTERY_URI)){
            child_list = get_child_list_for_battery();
            if (child_list == NULL){
                free_buffer(buffer);
                free_x(x_param);
                return MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
            }
        } else {
            child_list = get_child_list(nodes, nodeP->uri);
        }
        if (child_list) {
            nodeP->data_buffer = strdup(child_list);
        } else {
            nodeP->data_buffer = strdup(nodes[i].value);
        }

        if (nodeP->data_buffer) {
            nodeP->data_size = strlen(nodeP->data_buffer);
        }
        value = MO_ERROR_NONE;
    } else {
        //for OMADM_LEAF_FORMAT_CHAR
        if (!strcmp(nodeP->format, OMADM_LEAF_FORMAT_CHAR) ||
                !strcmp(nodeP->uri, STORAGE_URI)){
            switch (parameter){
                case WITHOUT_X:
                    getLeafFunc = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc) {
                        value = getLeafFunc(buffer);
                    } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    break;
                case X_BATTERY:
                    getLeafFunc_x = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc_x) {
                        value = getLeafFunc_x(x-1, buffer);
                    } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    break;
                default:
                    break;
            }

                while(value == RESULT_BUFFER_OVERFLOW){
                    char* old_data = buffer->data;
                    buffer->data = (char*)realloc(buffer->data, buffer->size);
                    if (buffer->data == NULL){
                        value = RESULT_MEMORY_ERROR;
                        free(old_data);
                        break;
                    }
                    memset(buffer->data, 0, sizeof(char) * buffer->size);
                    if (!parameter){
                        value = getLeafFunc(buffer);
                    } else value = getLeafFunc_x(x-1, buffer);
                }

                if (value == MO_ERROR_NONE) {
                    free(nodeP->data_buffer);
                        nodeP->data_buffer = strdup(buffer->data);
                        if(nodeP->data_buffer) {
                            nodeP->data_size = strlen(buffer->data);
                            err = MO_ERROR_NONE;
                        } else err = MO_ERROR_DEVICE_FULL;
                }
        } else if (!strcmp(nodeP->format, OMADM_LEAF_FORMAT_FLOAT)){
            //for OMADM_LEAF_FORMAT_FLOAT
            char data[10];
            switch (parameter){
                case WITHOUT_X:
                    getLeafFunc_float = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc_float) {
                        value = getLeafFunc_float(&float_buffer);
                    } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    break;
                case X_BATTERY:
                    getLeafFunc_float_x = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc_float_x) {
                        value = getLeafFunc_float_x(x-1, &float_buffer);
                    } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    break;
                case X_STORAGE:
                    getLeafFunc_float_storage = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc_float_storage) {
                        value = getLeafFunc_float_storage((const char*)x_param, &float_buffer);
                    } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    break;
                default:
                    break;
            }
            if (value == MO_ERROR_NONE) {
                free(nodeP->data_buffer);
                sprintf(data, "%0.1f", float_buffer);
                nodeP->data_buffer = strdup(data);
                if(nodeP->data_buffer) {
                    nodeP->data_size = strlen(data);
                    err = MO_ERROR_NONE;
                } else err = MO_ERROR_DEVICE_FULL;
            }
        } else if (!strcmp(nodeP->format, OMADM_LEAF_FORMAT_INT)){
            //for OMADM_LEAF_FORMAT_INT
            char data[10];
            switch (parameter){
                case WITHOUT_X:
                    getLeafFunc_int32 = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc_int32) {
                        value = getLeafFunc_int32(&int32_buffer);
                    } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    break;
                case X_BATTERY:
                    getLeafFunc_int32_x = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc_int32_x) {
                        value = getLeafFunc_int32_x(x-1, &int32_buffer);
                    } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    break;
                case X_STORAGE:
                    getLeafFunc_int_storage = dlsym(palHandle, nodes[i].getLeafFuncName);
                    if (getLeafFunc_int_storage) {
                        value = getLeafFunc_int_storage((const char*)x_param, &int32_buffer);
                    } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
                    break;
                default:
                    break;
            }

            if (value == MO_ERROR_NONE) {
                free(nodeP->data_buffer);
                sprintf(data, "%d", int32_buffer);
                nodeP->data_buffer = strdup(data);
                if(nodeP->data_buffer) {
                    nodeP->data_size = strlen(data);
                    err = MO_ERROR_NONE;
                } else err = MO_ERROR_DEVICE_FULL;
            }
        } else if (!strcmp(nodeP->format, OMADM_LEAF_FORMAT_BOOL)){
            //for OMADM_LEAF_FORMAT_BOOL
            getLeafFunc_int32 = dlsym(palHandle, nodes[i].getLeafFuncName);
            if (getLeafFunc_int32) {
                value = getLeafFunc_int32(&int32_buffer);
            } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;

            if (value == MO_ERROR_NONE) {
                free(nodeP->data_buffer);
                if (int32_buffer == 1){
                    nodeP->data_buffer = strdup("True");
                } else nodeP->data_buffer = strdup("False");
                if(nodeP->data_buffer) {
                    nodeP->data_size = strlen(nodeP->data_buffer);
                    err = MO_ERROR_NONE;
                } else err = MO_ERROR_DEVICE_FULL;
            }
        } else err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
    }

    if (err == MO_ERROR_NONE){
        if(nodeP->format) free(nodeP->format);
        nodeP->format = strdup(nodes[i].format);
        if(nodeP->format) {
            if (nodeP->type) free(nodeP->type);
            nodeP->type = strdup(nodes[i].type);
            if(!nodeP->type) {
                err = MO_ERROR_DEVICE_FULL;
            } else err = MO_ERROR_NONE;
        } else err = MO_ERROR_DEVICE_FULL;
        if (value != MO_ERROR_NONE){
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
    }

    free_buffer(buffer);
    free_x(x_param);
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
    int set_value = 0;
    int (*setLeafFunc_int)(int);

    if(nodeP == NULL || nodeP->data_buffer == NULL){
        return MO_ERROR_INCOMPLETE_COMMAND;
    }

    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;
    i = prv_find_node(nodes, nodeP->uri);
    if (i == -1) {
        return MO_ERROR_NOT_FOUND;
    }
    if(palHandle && nodes[i].setLeafFuncName) {
        //for OMADM_LEAF_FORMAT_INT, OMADM_LEAF_FORMAT_BOOL
        DM_LOGI("DiagMon_set = %s", nodeP->uri);
        set_value = atoi(nodeP->data_buffer);
        setLeafFunc_int = dlsym(palHandle, nodes[i].setLeafFuncName);
        if (setLeafFunc_int) {
            err = setLeafFunc_int(set_value);
            DM_LOGI("DiagMon_set_code_from_pal = %d", err);
            if(err != MO_ERROR_NONE){
                err = MO_ERROR_COMMAND_FAILED;
            }
        } else {
            err = MO_ERROR_COMMAND_NOT_IMPLEMENTED;
        }
    } else
        err = MO_ERROR_NOT_ALLOWED;

    return err;
}

static int prv_exec_fn (const char *iURI,
                       const char *cmdData,
                       const char *correlator,
                       void *iData)
{
    DM_LOGI("DIAGMON prv_exec_fn {\n");
    if (!palHandle){
       DM_LOGD( "ERROR! PAL isn't loaded. \n");
        return MO_ERROR_COMMAND_FAILED; /// seems that we can't do anything if PAL not loaded
    }
    if(iData == NULL) {
        DM_LOGD(" ERROR: invalid paraneters\n");
        return MO_ERROR_DEVICE_FULL;
    }
    int i = 0;
    int err = MO_ERROR_NONE;
    int value = RESULT_SUCCESS;
    int set_parameter = 2;
    int (*execLeafFunc)();
    int (*execLeafFunc_x)(int);
    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;

    if(iURI) {
        i = prv_find_node(nodes, iURI);
        if (i == -1) {
            return MO_ERROR_NOT_FOUND;
        }

        if(!strcmp(ALLOW_ROAMING, iURI) || !strcmp(MOBILE_DATA_ENABLE, iURI)){
            set_parameter = 1;
        } else if (!strcmp(DENY_ROAMING, iURI) || !strcmp(MOBILE_DATA_DISABLE, iURI)){
            set_parameter = 0;
        }
        if(palHandle && nodes[i].execLeafFuncName) {
            DM_LOGI("DiagMon_exec = %s", iURI);
            if (set_parameter == 2){
                execLeafFunc = dlsym(palHandle, nodes[i].execLeafFuncName);
                if (execLeafFunc) {
                    value = execLeafFunc();
                } else return MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
            } else {
                execLeafFunc_x = dlsym(palHandle, nodes[i].execLeafFuncName);
                if (execLeafFunc_x) {
                    value = execLeafFunc_x(set_parameter);
                } else return MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
            }
        } else return MO_ERROR_NOT_ALLOWED;
    }

    DM_LOGI("DiagMon_exec_code_from_pal = %d", value);
    if (value != MO_ERROR_NONE) {
        switch(value){
        case RESULT_MEMORY_ERROR:
            err = MO_ERROR_DEVICE_FULL;
            break;
        case RESULT_ERROR_INVALID_STATE:
            err = MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
            break;
        case RESULT_ERROR_UNDEFINED:
            err = MO_ERROR_COMMAND_FAILED;
        break;
        default:
            err = MO_ERROR_COMMAND_FAILED;
        }
    }
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
