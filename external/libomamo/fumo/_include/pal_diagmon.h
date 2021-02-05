/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * @file pal_diagmon.h
 * @brief File containing PAL interface for DiagMon plugin.
 *
 * Diagnostics Management.
 * The Diagnostics Management Object enables remote diagnostics management within Device.
 * @see http://openmobilealliance.org/
 */

#ifndef PAL_DIAGMON_H
#define PAL_DIAGMON_H
#include "pal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Network types.
 */
typedef enum {
    NETWORK_TYPE_UNKNOWN   = -1, /// Network type is unknown
    NETWORK_TYPE_1XRTT     = 0,  /// CDMA2000 1xRTT
    NETWORK_TYPE_EVDO_0    = 1,  /// CDMA2000 EVDO revision 0
    NETWORK_TYPE_EVDO_A    = 2,  /// CDMA2000 EVDO revision A
    NETWORK_TYPE_EHRPD     = 3,  /// EHRPD
    NETWORK_TYPE_GSM       = 4,  /// GSM
    NETWORK_TYPE_GPRS      = 5,  /// GPRS
    NETWORK_TYPE_EDGE      = 6,  /// EDGE (ETSI 27.007: "GSM w/EGPRS")
    NETWORK_TYPE_UMTS      = 7,  /// UMTS (ETSI 27.007: "UTRAN")
    NETWORK_TYPE_HSPA      = 8,  /// HSPA (ETSI 27.007: "UTRAN w/HSDPA and HSUPA")
    NETWORK_TYPE_HSPA_PLUS = 9,  /// HSPA+ (ETSI 27.007: "UTRAN w/HSPA+")
    NETWORK_TYPE_LTE       = 10  /// LTE (ETSI 27.007: "E-UTRAN")
} network_type_t;

/**
 * SIM states.
 */
typedef enum {
    SIM_STATE_UNKNOWN        = 0, /// SIM is in transition between states
    SIM_STATE_ABSENT         = 1, /// no SIM card is available in the device
    SIM_STATE_PIN_REQUIRED   = 2, /// requires the user's SIM PIN to unlock
    SIM_STATE_PUK_REQUIRED   = 3, /// requires the user's SIM PUK to unlock
    SIM_STATE_NETWORK_LOCKED = 4, /// requires a network PIN to unlock
    SIM_STATE_READY          = 5  /// SIM card state: Ready
} sim_state_t;

/**
 * Connection types
 */
typedef enum {
    CONNECT_TYPE_WIFI    = 0,
    CONNECT_TYPE_MOBILE  = 1,
    CONNECT_TYPE_UNKNOWN = 2
} connect_type_t;

/**
 * State of Network (Home/Roaming)
 */
typedef enum {
    PAL_NETWORKTYPE_HOMEROAMING_UNKNOWN,
    PAL_3GPP_HOME,
    PAL_3GPP_ROAMING,
    PAL_CDMA_HOME,
    PAL_CDMA_ROAMING
} networktype_homeroaming_t;

/**
 * Get Home / Roaming status with network type
 * @return Status
 */
networktype_homeroaming_t pal_networktype_homeroaming(void);

/**
 * Reads type of the current system which is used for Voice Traffic
 * to the p_value pointer
 * @see ./ManagedObjects/DiagMon/RF/CurrentSystem/Voice
 * @param[out] out
 * @parblock
 * pointer to store type of the current system which is used for Voice Traffic
 *  "Voice not available"
 *  "1xRTT"
 *  "LTE"
 *  "WiFi"
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_curr_voice_get(data_buffer_t* out);

/**
 * Reads type of the current system which is used for Data access
 * to the p_value pointer
 * @see ./ManagedObjects/DiagMon/RF/CurrentSystem/Data
 * @param[out] out
 * @parblock
 * pointer to store type of the current system which is used for Data access
 *   "mobile data is not available"
 *   "1xRTT"
 *   "eHPRD"
 *   "LTE"
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_curr_data_get(data_buffer_t* out);

/**
 * Reads flag is device in roaming network
 * to the p_value pointer
 * @see ./ManagedObjects/DiagMon/RF/HomeRoam
 * @param[out] out
 * @parblock
 * pointer to store flag is device in roaming network
 *   "Home"
 *   "Roam"
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_homeroam_get(data_buffer_t* out);

/**
 * Reads 1x Signal strength value in dBm to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/1xSignal
 * @param[out] p_value 1x Signal strength value in dBm
 * @return @see enum result_states
 */
int pal_network_1xsignal_get(float* p_value);

/**
 * Reads 4G Signal strength value in dBm to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/4GSignal
 * @param[out] p_value 4G Signal strength value in dBm
 * @return @see enum result_states
 */
int pal_network_4gsignal_get(float* p_value);

/**
 * Reads base station identification number
 * @see ./ManagedObjects/DiagMon/Network/BaseStationId
 * @param[out] p_value base station identification number
 * @return @see enum result_states
 */
int pal_network_basestationid_get(int32_t* p_value);

/**
 * Reads CDMA System ID to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/SystemID
 * @param[out] p_value CDMA System ID
 * @return @see enum result_states
 */
int pal_network_systemid_get(int32_t* p_value);

/**
 * Reads the operator MCC to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/MCC
 * @param[out] p_value the operator MCC value
 * @return @see enum result_states
 */
int pal_network_mcc_get(data_buffer_t* p_value);

/**
 * Reads CDMA network identification number to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/NetworkID
 * @param[out] p_value network Id
 * @return @see enum result_states
 */
int pal_network_networkid_get(int32_t* p_value);

/**
 * Reads array of supported preferred network modes to the outbuf->data buffer
 * as array of int32_t values
 * @see ./ManagedObjects/DiagMon/Network/Mode/SupportedModes
 * @param[out] outbuf
 * @parblock
 *  ((int32_t*)(outbuf->data)): array of supported network modes
 *   0 : Global Mode
 *   1 : LTE/CDMA Mode
 *   2 : GSM/UMTS Mode
 *  ((int32_t)(outbuf->size)): of array of supported network modes in bytes
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_supported_modes_get(data_buffer_t* outbuf);

/**
 * Reads current preferred network mode to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/Mode/PreferredNetworkMode
 * @param[out] p_value
 * @parblock
 * pointer to store current preferred network mode:
 *   0 : Global Mode
 *   1 : LTE/CDMA Mode
 *   2 : GSM/UMTS Mode
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_preferred_network_mode_get(int32_t* p_value);

/**
 * Reads flag "is data enabled in roaming" to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/GlobalDataRoamingAccess
 * @param[out] p_value
 * @parblock
 * pointer to store flag "is data enabled in roaming":
 *   1 : enabled
 *   0 : disabled
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_global_data_roaming_access_get(int32_t* p_value);

/**
 * Reads the operator MNC to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/MNC
 * @param[out] p_value the operator MNC value
 * @return @see enum result_states
 */
int pal_network_mnc_get(data_buffer_t* p_value);

/**
 * Reads 3G Signal strength value in dBm to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/3GSignal
 * @param[out] p_value 3G Signal strength value in dBm
 * @return @see enum result_states
 */
int pal_network_3gsignal_get(float* p_value);

/**
 * Reads current mobile network type to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/CurrentNetwork
 * @param[out] p_value
 * @parblock
 * pointer to store current mobile network type:
 *   0 : NETWORK_TYPE_1xRTT
 *   1 : NETWORK_TYPE_EVDO_0
 *   2 : NETWORK_TYPE_EVDO_A
 *   3 : NETWORK_TYPE_EHRPD
 *   4 : NETWORK_TYPE_GSM
 *   5 : NETWORK_TYPE_GPRS
 *   6 : NETWORK_TYPE_EDGE
 *   7 : NETWORK_TYPE_UMTS
 *   8 : NETWORK_TYPE_HSPA
 *   9 : NETWORK_TYPE_HSPA_PLUS
 *  10 : NETWORK_TYPE_LTE
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_currentnetwork_get(int32_t* p_value);

/**
 * Reads current device call state to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/CallState
 * @param[out] p_value
 * @parblock
 * pointer to store current device call state
 *   0 : idle
 *   1 : ringing
 *   2 : offhook
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_callstate_get(int32_t* p_value);

/**
 * Reads current active default data network type to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/ConnectType
 * @param[out] p_value
 * @parblock
 * pointer to store active default data network type:
 *   0 : WI-FI
 *   1 : Mobile
 *   2 : Unknown
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_connecttype_get(int32_t* p_value);

/**
 * Reads ISO country code to the outbuf->data buffer as C-string (char*)
 * @param[out] outbuf buffer "outbuf->data" contains country ISO code as C-string
 * @see ./ManagedObjects/DiagMon/Network/NetworkCountryIso
 * @return @see enum result_states
 */
int pal_network_network_countryiso_get(data_buffer_t* outbuf);

/**
 * Reads device SIM state to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/SIMState
 * @param[out] p_value
 * @parblock
 * pointer to store device SIM state:
 *   0 : unknown
 *   1 : absent
 *   2 : pin required
 *   3 : puk required
 *   4 : network locked
 *   5 : ready
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_simstate_get(int32_t* p_value);

/**
 * Reads number of batteries to the p_value pointer
 * @param[out] p_value *p_value contains number of batteries
 * @see ./ManagedObjects/DiagMon/Battery
 * @return @see enum result_states
 */
int pal_system_batteries_get(int32_t* p_value);

/**
 * Reads status of Mobile Data to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/MobileData/Enabled
 * @param[out] p_value
 * @parblock
 * pointer to store status of Mobile Data:
 *   0 : mobile data is disabled
 *   1 : mobile data is enabled
 * @endparblock
 * @return : @see enum result_states
 */
int pal_network_mobiledata_state_get(int32_t* p_value);

/**
 * Reads current battery temperature to the iobuf->data buffer as C-string
 * @param[in] battery_idx battery number
 * @param[out] iobuf
 * @parblock
 *   outbuf->data buffer contains C-string
 *   example: "78 F" or "26 C"
 * @endparblock
 * @see ./ManagedObjects/DiagMon/Battery/<X>/Temp
 * @return @see enum result_states
 */
int pal_system_battery_x_temp_get(int battery_idx, data_buffer_t* iobuf);

/**
 * Reads current battery level to the p_value pointer
 * @param[in] battery_idx number of battery
 * @param[out] out battery level in can ve from 0.0f to 100.0f
 * @see ./ManagedObjects/DiagMon/Battery/<X>/Level
 * @return @see enum result_states
 */
int pal_system_battery_x_level_get(int battery_idx, float* out);

/**
 * Reads current battery status to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Battery/<X>/Status
 * @param[in] battery_idx number of battery
 * @param[out] p_value
 * @parblock
 * pointer to store current battery status
 *   0 : Charging
 *   1 : Discharging
 *   2 : Charging Complete
 *   3 : Unknown
 *   4 : Not Installed
 * @endparblock
 * @return : @see enum result_states
 */
int pal_system_battery_x_status_get(int battery_idx, int32_t* p_value);

/**
 * Reads battery type to the iobuf->data buffer as C-string
 * @param[in] battery_idx number of battery
 * @param[out] iobuf iobuf->data contains type of selected battery as C-string
 * @see ./ManagedObjects/DiagMon/Battery/<X>/Type
 * @return : @see enum result_states
 */
int pal_system_battery_x_type_get(int battery_idx, data_buffer_t* iobuf);

/**
 * Reads current voltage of battery in mV
 * @see ./ManagedObjects/DiagMon/Battery/<X>/Voltage
 * @param[in] battery_idx number of battery
 * @param[out] p_value *p_value : current voltage of battery in mV
 * @return : @see enum result_states
 */
int pal_system_battery_x_voltage_get(int battery_idx, int32_t* p_value);

/**
 * Reads plug status to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Battery/<X>/Plug_Status
 * @param[in] battery_idx number of battery
 * @param[out] p_value
 * @parblock
 * pointer to store current battery status
 *   0 : AC
 *   1 : USB
 *   2 : Wireless
 *   3 : Other
 *   4 : Unplugged
 *   5 : Unknown
 * @endparblock
 * @return : @see enum result_states
 */
int pal_system_battery_x_plug_status_get(int battery_idx, int32_t* p_value);

/**
 * Reads available RAM to the p_mbs
 * @param[out] p_mbs memory in MiB
 * @see ./ManagedObjects/DiagMon/Memory/Avail
 * @return : @see enum result_states
 */
int pal_system_memory_avail_get(float* p_mbs);

/**
 * type of pointer to the function "int function(float*)"
 */
typedef int (*pal_int_pr4_fn) (float*);

/**
 * Reads percent available RAM to the percents
 * @param[out] percents percents of available RAM
 * @see ./ManagedObjects/DiagMon/Memory/PercentFree
 * @return : @see enum result_states
 */
int pal_system_memory_percentfree_get(int* percents);

/**
 * Reads size of RAM to the p_mbs
 * @see ./ManagedObjects/DiagMon/Memory/Total
 * @param[out] p_mbs memory in MiB
 * @return @see enum result_states
 */
int pal_system_memory_total_get(float* p_mbs);

/**
 * Reads size of ROM to the p_mbs
 * @param[in]path path on filesystem
 * @param[out]p_bs amount of memory
 * @return @see enum result_states
 */
int pal_system_get_memory_free_by_path(const char* path, unsigned long long* p_bs);

/**
 * Enable or disable data connection in roaming
 * @param[in] enable
 * @parblock
 *   0 : disable data connection in roaming
 *   1 : enable data connection in roaming
 * @endparblock
 * @see ./ManagedObjects/DiagMon/Network/Operations/DenyRoaming
 * @see ./ManagedObjects/DiagMon/Network/Operations/AllowCurrentTripRoaming
 * @return @see enum result_states
 */
int pal_network_global_data_roaming_access_set(int enable);

/** Global preferred network mode */
#define PAL_SYSTEM_NW_MODE_CLOBAL   0x00000000
/** LTE CDMA preferred network mode */
#define PAL_SYSTEM_NW_MODE_LTE_CDMA 0x00000001
/** GSM UMTS preferred network mode */
#define PAL_SYSTEM_NW_MODE_GSM_UMTS 0x00000002

/**
 * Sets preferred network mode
 * @param[in] nw_mode
 * @parblock
 *   preferred network mode
 *   0 : ./ManagedObjects/DiagMon/Network/Mode/Operations/GlobalMode
 *   1 : ./ManagedObjects/DiagMon/Network/Mode/Operations/LTE_CDMA_Mode
 *   2 : ./ManagedObjects/DiagMon/Network/Mode/Operations/GSM_UMTS_Mode
 * @endparblock
 * @see ./ManagedObjects/DiagMon/Network/Mode/Operations/GlobalMode
 * @see ./ManagedObjects/DiagMon/Network/Mode/Operations/LTE_CDMA_Mode
 * @see ./ManagedObjects/DiagMon/Network/Mode/Operations/GSM_UMTS_Mode
 * @return @see enum result_states
 */
int pal_network_preferred_network_mode_set(int nw_mode);

/**
 * Enables or disables mobile data connection.
 * @param[in] mobile_data_enable
 * @parblock
 *   0 : disable mobile data
 * @see ./ManagedObjects/DiagMon/Network/MobileData/Operations/Disable
 *   1 : enable mobile data
 * @see ./ManagedObjects/DiagMon/Network/MobileData/Operations/Enable
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_mobiledata_state_set(int mobile_data_enable);

/**
 * Reads list of mounted storages
 * @see ./ManagedObjects/DiagMon/Storage
 * @param[out] list buffer with slash separated device names
 * example: "SDCard0/SDCard1/SDCard2/USB2.0/USB3.0/Other1/Other2"
 * @return @see enum result_states
 */
int pal_system_storages_get(data_buffer_t* list);

/**
 * Describes information about space on the file system.
 */
typedef struct {
  /// size of block in bytes
  int64_t block_size;
  /// quantity of blocks
  int64_t blocks;
} fsinfo_t;

/**
 * Reads free space on the storage
 * @param[in] name name of mounted storage
 * @param[out] mbs information about free space on the file system
 * in MiBs
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Avail
 * @return @see enum result_states
 */
int pal_system_storage_x_avail_get(const char* name, float* mbs);

/**
 * Reads size of the storage to the fsinfo pointer
 * @param[in] name name of mounted storage
 * @param[out] mbs information about size of storage in MiBs
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Total
 * @return @see enum result_states
 */
int pal_system_storage_x_total_get(const char* name, float* mbs);

/**
 * Reads size of used space on the mounted storage
 * @param[in] name name of mounted storage
 * @param[out] mbs information about used space on the mounted
 * storage in MiBs
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Used
 * @return @see enum result_states
 */
int pal_system_storage_x_used_get(const char* name, int32_t* mbs);

/**
 * Reads size of the used disk space by applications
 * @param mount_point_path : mount point
 * @param[in] name name of mounted storage
 * @param[out] mbs information about used space by applications on the mounted
 * storage in MiBs
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Used/Applications
 * @return @see enum result_states
 */
int pal_system_storage_x_used_applications_get(
    const char* name, int32_t* mbs);

/**
 * Reads size of the used disk space by video
 * and pictures on the selected storage
 * @param[in] name name of mounted storage
 * @param[out] mbs information about used space by video and pictures in MiBs
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Used/Pictures_Video
 * @return @see enum result_states
 */
int pal_system_storage_x_used_pictures_video_get(
    const char* name, int32_t* mbs);

/**
 * Reads size of the used disk space by audio files on the selected storage
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Used/Audio
 * @param[in] name name of mounted storage
 * @param[out] mbs space used by audio files in MiBs
 * @return @see enum result_states
 */
int pal_system_storage_x_used_audio_get(const char* name, int32_t* mbs);

/**
 * Reads size of the used disk space by downloaded files on the selected storage
 * @param[in] name name of mounted storage
 * @param[out] mbs space used by downloads files in MiBs
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Used/Downloads
 * @return @see enum result_states
 */
int pal_system_storage_x_used_downloads_get(const char* name, int32_t* mbs);

/**
 * Reads size of the used disk space by miscellaneous files
 * on the selected storage
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Used/Miscellaneaous
 * @param[in] name name of mounted storage
 * @param[out] mbs space used by miscellaneous in MiBs
 * @return @see enum result_states
 */
int pal_system_storage_x_used_miscellaneaous_get(
    const char* name, int32_t* mbs);

/**
 * Reads size of the used disk space by cache on the selected storage
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Used/Cache
 * @param[in] name name of mounted storage
 * @param[out] mbs space used by caches in MiBs
 * @return @see enum result_states
 */
int pal_system_storage_x_used_caches_get(const char* name, int32_t* mbs);

/**
 * Reads free space on the storage
 * @param[in] name name of mounted storage
 * @param[out] percents information about free space on the file system
 * in percents
 * @see ./ManagedObjects/DiagMon/Storage/<X>/Avail
 * @return @see enum result_states
 */
int pal_system_storage_percentfree_get(const char* name, int* percents);

/**
  * Gets WiFi status of the device.
  *
  * @param[out] outdata WiFi status of the device (1 - WiFi is Enabled, 0 - WiFi is Disabled)
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_wifi_state_get(int *outdata);

/**
  * Gets the speed in Mbps at which the WiFi connection is connected at.
  *
  * @param[out] outdata
  * @parblock
  *  WiFI current link speed in Mbps in integer format,
  *
  *  0 if device is not connected to a WiFi network
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_wifi_speed_get(int *outdata);

/**
  * Gets the detailed state of the supplicant's negotiation with an access point.
  *
  * @param[out] outdata
  * @parblock
  *  state of WiFi adapter, possible values:
  *
  *           0 ASSOCIATED
  *           1 ASSOCIATING
  *           2 AUTHENTICATING
  *           3 COMPLETED
  *           4 DISCONNECTED
  *           5 DORMANT
  *           6 FOUR_WAY_HANDSHAKE
  *           7 GROUP_HANDSHAKE
  *           8 INACTIVE
  *           9 INTERFACE_DISABLED
  *           10 INVALID
  *           11 SCANNING
  *           12 UNINITIALIZED
  * @endparblock
  * @return an error code @see enum result_states
  *
  */
int pal_network_wifi_status_get(int *outdata);

/**
  * Returns the SSID of each of the Wi-Fi networks that are visible to the device
  * along with the RSSI values for those networks and the Network Security option.
  *
  * @param[out] outbuf - output data in format:
  * @parblock
  *
  *  size    size of received data in bytes
  *
  *  data    represents SSID,RSSI and Network Security option of networks that are visible
  *
  *  Format of outbuf->data:
  *  -Multiple networks are separated by "++" (two plus signs without quotes);
  *  -SSID, RSSI value, and Security Option for one Network are separated by "," (comma without quotes);
  *  -Multiple Security options are separated by "_";
  *  -NO extra blank spaces between or around the values;
  *  -SSID name(s) are exactly (case and format preserved) as they are sent by the Access Point with Blank spaces in SSID Name(s) if exsist;
  *  -For RSSI values, only the numbers are reported with maximum of 1 decimal place. The word dBm is not reported as part of the result;
  *  -For Network security option, possible values are:
  *    0 Unsecure
  *    1 WEP
  *    2 WPA
  *    3 WPA2 Personal
  *    4 WPA2 Enterprise
  *
  *  For example, if 3 networks are visible and:
  *  If the SSID names are as follows: Sample SSID1, Sample_SSID2, and SampleSSID3
  *  And their respective RSSI values are: -90dBm, -80.5dBm, -75dBm
  *  And their respective Network Security options are: Non-Secure, WEP Protected, and (WEP + WPA + WPA2 personal)
  *  Then, the response from the device is: Sample SSID1,-90,0++Sample_SSID2,-80.5,1++SampleSSID3,-75,1_2_3
  *
  *  If no networks are available, then the return value is N/A,N/A,N/A for SSID, RSSI, and Security Option.
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_wifi_networks_get(data_buffer_t *outbuf);

/**
  * Gets the received signal strength indicator of the current WiFi network (expressed in dBm).
  *
  * @param[out] outdata
  * @parblock
  *  Received Signal Strength in dBm in integer format,
  *
  *  0 if device is not connected to a WiFi network
  * @endparblock
  * @return an error code @see enum result_states
  *
  */
int pal_network_wifi_signal_get(int *outdata);

/**
  * Gets the service set identifier (SSID) of the current WiFi Network device is connected to.
  *
  * @param[out] outbuf
  * @parblock
  *
  *  size    size of received data in bytes
  *
  *  data    value of case sensitive broadcasted SSID of the current WiFi Network
  *
  *          "N/A" if device is not connected to a WiFi network
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_wifi_ssid_get(data_buffer_t *outbuf);

/**
  * Returns the status of Device's Hotspot functionality.
  *
  * @param[out] outdata state of Device's Hotspot (1 - Hotspot is Enabled, 0 - Hotspot is Disabled)
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_wifi_hotspot_state_get(int *outdata);

/**
  * Returns the MAC address of the WiFi adapter of the device.
  *
  * @param[out] outbuf
  * @parblock
  *
  *  size    size of received data in bytes
  *
  *  data    MAC address in format XX:XX:XX:XX:XX:XX, all letters are in upper case.
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_system_wifi_mac_get(data_buffer_t *outbuf);

/**
  * Gets the Basic Service Set ID value of the Access point the device is connected to.
  * This is the MAC address of the Access Point that the device is connected to
  *
  * @param[out] outbuf
  * @parblock
  *
  *  size    size of received data in bytes
  *
  *  data    MAC address in format XX:XX:XX:XX:XX:XX, all letters are in upper case
  *
  *          "00:00:00:00:00:00" if the device is not connected to an Access Point
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_wifi_bssid_get(data_buffer_t *outbuf);

/**
  * Gets the Bluetooth Name of the Device.
  *
  * @param[out] outbuf
  * @parblock
  *
  *  size    size of received data in bytes
  *
  *  data    Bluetooth Name.
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_name_get(data_buffer_t *outbuf);

/**
  * Gets the status of the Bluetooth on the Device.
  *
  * @param[out] outdata status of Bluetooth (1 - Bluetooth is Enabled, 0 - Bluetooth is Disabled)
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_state_get(int *outdata);

/**
  * Gets the state of the Health Device Profile (HDP) of the device.
  * Note: HDP - Profile designed to facilitate transmission and reception of Medical Device data
  *
  * @param[out] outdata
  * @parblock
  *  state of HDP of the device, possible values:
  *
  *           0 CONNECTED
  *           1 CONNECTING
  *           2 DISCONNECTED
  *           3 DISCONNECTING
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_hdp_get(int *outdata);

/**
  * Gets the device state of Advanced Audio Distribution profile (A2DP).
  * Note: A2DP - profile defines how multimedia audio can be streamed from one device to another over a Bluetooth connection
  *
  * @param[out] outdata
  * @parblock
  *  A2DP state of the device, possible values:
  ^
  *           0 CONNECTED
  *           1 CONNECTING
  *           2 DISCONNECTED
  *           3 DISCONNECTING
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_a2dpstate_get(int *outdata);

/**
  * Gets the Headset profile (HSP) state of the device.
  * Note: HSP profile, providing support for the popular Bluetooth Headsets to be used with mobile phones
  *
  * @param[out] outdata
  * @parblock
  *  HSP state of the device, possible values:
  *
  *           0 CONNECTED
  *           1 CONNECTING
  *           2 DISCONNECTED
  *           3 DISCONNECTING
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_hspstate_get(int *outdata);

/**
  * Gets the status of Discoverable Mode for Bluetooth on the device.
  *
  * @param[out] outdata status of Discoverable Mode (1 - Discoverable Mode is Discoverable, 0 - Discoverable Mode is Not Discoverable)
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_discoverable_state_get(int *outdata);

/**
  * Gets the status of Standalone GPS on the device.
  *
  * @param[out] outdata status of Standalone GPS (1 - Standalone GPS is Enabled, 0 - Standalone GPS is Disabled)
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_gps_state_get(int *outdata);

/**
  * Gets how many Satellites are currently visible to the device.
  *
  * @param[out] outdata
  * @parblock
  *  a number of Satellites are currently visible to the device in integer format
  *
  *          0 if GPS is disabled
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_gps_num_of_satellites_get(int *outdata);

/**
  * Gets the Signal to Noise ration observed for Satellite signal.
  *
  * @param[out] outdata
  * @parblock
  *  Signal to Noise ration observed for Satellite signal in float format
  *
  *          0 if GPS is disabled
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_gps_snr_inviewsatellites_get(float *outdata);

/**
  * Gets the NFC status on the device.
  *
  * @param[out] outdata NFC status (1 - NFC status is Enabled, 0 - NFC status is Disabled)
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_nfc_state_get(int *outdata);

/**
  * Gets the Device Encryption status.
  *
  * @param[out] outdata Device Encryption status (1 - Device Encryption is Enabled, 0 - Device Encryption is Disabled)
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_encryption_state_get(int *outdata);

/**
  * Gets the App Verification status.
  *
  * @param[out] outdata App Verification status (1 - App Verification is Enabled, 0 - App Verification is Disabled)
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_verifyapp_state_get(int *outdata);

/**
  * Sets App Verification to Enable on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_verifyapp_enable();

/**
  * Gets the current Ringtone volume level on the device.
  *
  * @param[out] outdata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_volumes_ringtone_get(int *outdata);

/**
  * Sets the current Ringtone volume level on the device.
  *
  * @param[in] indata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_volumes_ringtone_set(int indata);

/**
  * Gets the current Notification volume level on the device.
  *
  * @param[out] outdata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_volumes_notifications_get(int *outdata);

/**
  * Sets Notification volume level on the device.
  *
  * @param[in] indata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_volumes_notifications_set(int indata);

/**
  * Gets the current Alarms volume level on the device.
  *
  * @param[out] outdata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_diagmon_sound_volumes_alarms_get(int *outdata);

/**
  * Sets Alarms volume level on the device.
  *
  * @param[in] indata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_volumes_alarms_set(int indata);

/**
  * Gets the current Audio/Video/Media volume level on the device.
  *
  * @param[out] outdata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_volumes_media_get(int *outdata);

/**
  * Sets Audio/Video/Media volume level on the device.
  *
  * @param[in] indata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_volumes_media_set(int indata);

/**
  * Gets the Bluetooth volume level that is set by the device during a phone call.
  *
  * @param[out] outdata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_volumes_bluetooth_get(int *outdata);

/**
  * Sets Bluetooth volume level on the device.
  *
  * @param[in] indata
  * @parblock
  *  value in range (0, 25, 50, 75, 100), where:
  *
  *           0   0%(vibrate)
  *           25  25%
  *           50  50%
  *           75  75%
  *           100 100%
  * @endparblock
  *
  * @return an error code @see enum result_states
  *
  */
int pal_diagmon_sound_volumes_bluetooth_set(int indata);

/**
  * Sets WiFi mode to Enabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_wifi_enable();

/**
  * Sets WiFi mode to Disabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_wifi_disable();

/**
  * Sets HotSpot to Disabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_wifi_hotspot_disable();

/**
  * Set the Network Mode to Global Mode.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_globalmode_enable();

/**
  * Set the Network Mode to LTE/CDMA Mode.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_lte_cdma_mode_enable();

/**
  * Set the Network Mode to GSM/UMTS Mode.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_network_gsm_umts_mode_enable();

/**
  * Sets Bluetooth to Enabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_enable();

/**
  * Sets Bluetooth to Disabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_disable();

/**
  * Sets Discoverable mode for Bluetooth to Enabled on the device and automatically enable Bluetooth if Bluetooth was disabled.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_discoverable_enable();

/**
  * Sets Discoverable mode for Bluetooth to Disabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_bluetooth_discoverable_disable();

/**
  * Sets Standalone GPS mode Enabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_gps_enable();

/**
  * Sets Standalone GPS mode Disabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_gps_disable();

/**
  * Sets NFC status to Enabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_nfc_enable();

/**
  * Sets NFC status to Disabled on the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_nfc_disable();

/**
  * Gets NFC status of the device.
  *
  * @return an error code @see enum result_states
  *
  */
int pal_device_nfc_state_get(int *outdata);

#ifdef __cplusplus
}
#endif

#endif // PAL_DIAGMON_H
