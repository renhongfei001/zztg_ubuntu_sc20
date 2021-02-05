/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef NET_MANAGER_H
#define NET_MANAGER_H
/*!
 * @file network_manager.h
 *
 * @brief Manager of celluar network status
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "pal.h"

/*!
 *  The data network status
 */

typedef struct
{
    bool isWiFiConnected;
    bool isCellularConnected; // is actual for any cellular data connection such as
                              // LTE or 3G if possible
    bool isDataRoaming;
    bool isActiveVoiceCall;   // is actual for active voice call
} network_conditions_state_t;

/**
 * @brief Initializes network manager and all its components
 * @return true in case of successful initialization; false - otherwise
 */
bool netm_init_network_manager();

/**
 * @brief Deinitializes network manager
 */
void netm_stop_network_manager(void);

/**
 * Check the wifi data connection
 * @return true - if device is connected to wifi network,
 *         false - otherwise
 */
bool netm_is_wifi_connected();

/**
 * @brief Setup admin apn network connection
 */
void netm_up_admin_network();

/**
 * @brief Release admin apn network connection
 */
void netm_down_admin_network();

/*!
 * Check the available data connection
 * @return status of available data connection:
 *     true  - network data connection is available,
 *     false - network data connection is not available.
 */
bool netm_is_network_ready();

/*!
 * Request the ability of data connection.
 * The request will locked until data connection is appears.
 * @return status of available data connection:
 *     true  - network data connection is available,
 *     false - in error case
 */
bool netm_is_network_ready_lock();

/**
 * Process celluar network state events form PAL
 * @param[in] event  celluar network
 * @return Result code
 */
int netm_handler_process_event(network_conditions_t event);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NET_MANAGER_H
