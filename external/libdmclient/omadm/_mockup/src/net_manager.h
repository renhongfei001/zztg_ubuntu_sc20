/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <stdbool.h>

/**
 * It check the wifi data connection
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

#endif // NET_MANAGER_H
