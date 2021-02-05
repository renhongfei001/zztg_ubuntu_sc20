/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * @file pal_common.h
 * @brief File containing PAL common definitions and errors.
 */

#ifndef PAL_COMMON_H
#define PAL_COMMON_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Structure which is used to exchange data with PAL library
 */
typedef struct {
    /**
     * Calling function stores size of the allocated memory
     * in this value.
     * Called function stores size of the data which should be stored
     * in the buffer.
     */
    unsigned int size;
    /**
     * Called function stores data in this buffer
     * if size which is set by calling function is enough.
     */
    char *data;
} data_buffer_t;

/// OMADM request types
typedef enum {
    PACKAGE0,         ///< wap push or UI type
    GENERIC_ALERT,    ///< management generic object alert type
    SESSION_ALERT,    ///< management object alert type
    SU_CANCEL,        ///< SU cancel sms type
    USER_SYS_UPDATE,  ///< start system update by user request
    START_AUTOSESSION,///< start system update by autosession request
    UDM_REQUEST,      ///< start system update by udm request
} omadm_request_type_t;

/**
 * Omadm Callback functions identification
 */
typedef enum {
    OMADM_FUMO_USER_REPLY = 100,
    OMADM_FUMO_GET_SYSTEM_UPDATE_INFO = 101,
    OMADM_CONTROLLER_USER_REPLY = 102,
    OMADM_ADMIN_NETWORK_STATUS = 103,
    OMADM_NET_MANAGER_REPLY = 104,
    OMADM_SCOMO_GET_BATTERY_STATE = 105,
    OMADM_FUMO_GET_BATTERY_STATE = 106,
    OMADM_FUMO_CHECK_FOR_UPDATE = 107,
    OMADM_UNUSED = 200
} OmadmCallbackId;

// Declare callback
typedef int omadmCallback_id;
typedef void (*omadmCallback)(void*);

////////////////////////////////////////////////////////////////////////////////
///    network manager section
////////////////////////////////////////////////////////////////////////////////

/// Defines of status for Admin network
#define ADMIN_NET_UNKNOWN     0
#define ADMIN_NET_AVAILABLE   1
#define ADMIN_NET_UNAVAILABLE 2
#define ADMIN_NET_LOST        3
// timeout in seconds to establish admin network connection
#define ADMIN_NET_SETUP_TIMEOUT 5

/**
 * type of data network state
 */
typedef enum
{
    NETWORK_WIFI_CONNECTED = 0,
    NETWORK_CELLULAR_CONNECTED, // is actual for any cellular data connection
                                // such as LTE or 3G if possible
    NETWORK_DATA_ROAMING,
    NETWORK_ACTIVE_VOICE_CALL  // is actual for active voice call
} network_conditions_type_t;
/**
 *  @brief network event description
 */
typedef struct
{
    network_conditions_type_t net_feature;
    bool enabled;
} network_conditions_t;

/**
 * @brief Battery status possible values.
 */
#define BATTERY_STATUS_UNKNOWN        1;
#define BATTERY_STATUS_CHARGING       2;
#define BATTERY_STATUS_DISCHARGING    3;
#define BATTERY_STATUS_NOT_CHARGING   4;
#define BATTERY_STATUS_FULL           5;

/**
 * @brief parameter for battery status omadm callback
 * function.
 */
typedef struct {
    /**
     * @breif battery status.
     * [in]
     */
    int battery_status;
    /**
     * @breif battery level.
     * [in]
     */
    int battery_level;
} pal_battery_state;

#ifdef __cplusplus
}
#endif

#endif // PAL_COMMON_H
