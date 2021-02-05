/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * @file pal_devinfo.h
 * @brief File containing PAL interface for DevInfo plugin.
 *
 * Device Information.
 * Device information for the OMA DM server. Sent from the client to the server
 * for operation of the OMA DM protocol. The DevInfo object is sent from client
 * to server in the beginning of every session. Mandatory plugin.
 * @see http://openmobilealliance.org/
 */

#ifndef PAL_DEVINFO_H
#define PAL_DEVINFO_H
#include "pal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 * @param[out] Buffer an unique device identifier such as MAC of network card or IMEI
 * For Android device it is IMEI
 * IMEI comes form TelephonyManager service
 */
int pal_system_dev_id_get(data_buffer_t *Buffer);

/**
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 * @param[out] Buffer manufacturer name
 * For Android device it is a brand of this device.
 * For example: Verizon Phone (M123) -> Verizon
 * Manufacturer name comes from fimware build data
 */
int pal_system_man_get(data_buffer_t *Buffer);

/**
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 * @param[out] Buffer model name
 * For Android device it is a name of this device.
 * For example:  Verizon Phone (M123) -> M123
 * Manufacturer name comes from fimware build data
 */
int pal_system_mod_get(data_buffer_t *Buffer);

/**
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 * @param[out] Buffer version of OMDMA protocol
 * For Android device it is a version of current OMDMA protocol
 * This is hardcoded value "1.2"
 */
int pal_system_dmv_get(data_buffer_t *Buffer);

/**
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 * @param[out] Buffer current default system locale
 * For Android device it is a language, what used for user's applications
 * and OS translation
 * Comes from Locale
 */
int pal_system_lang_get(data_buffer_t *Buffer);

/**
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 * @param[out] Buffer sim serial number
 * For Android device this value is getting from telephony manager.
 */
int pal_system_iccid_get(data_buffer_t *Buffer);

#ifdef __cplusplus
}
#endif

#endif // PAL_DEVINFO_H
