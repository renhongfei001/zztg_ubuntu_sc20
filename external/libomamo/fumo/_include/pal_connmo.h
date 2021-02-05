/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * @file pal_connmo.h
 * @brief File containing PAL interface for ConnMO plugin.
 *
 * Connectivity Management.
 * Providing a standardized set of management objects for configuration of
 * data network connectivity through the OMA Device Management system will
 * improve the usability and customer experience of mobile terminals that rely
 * upon data services.
 * @see http://openmobilealliance.org/
 */

#ifndef PAL_CONNMO_H
#define PAL_CONNMO_H
#include "pal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Reads ID of the APN with class 1
 *
 * @param[out] obuf obuf->data contains id for class 1 apn as c-string
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class1_id_get(data_buffer_t *Buffer);

/**
 * Reads name for class 1 apn.
 *
 * @param[out] obuf obuf->data contains name value for class 1 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class1_name_get(data_buffer_t *Buffer);

/**
 * Sets name for class 1 apn.
 *
 * @param[in] ibuf ibuf->data contains new name value
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class1_name_set(data_buffer_t *Buffer);

/**
 * Reads ip version for class 1 apn.
 *
 * @param[out] obuf obuf->data contains ip value for class 1 apn
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class1_ip_get(data_buffer_t *Buffer);

/**
 * Sets ip version for class 1 apn.
 *
 * @param[in] ibuf ibuf->data contains new ip value
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class1_ip_set(data_buffer_t *Buffer);

/**
 * Reads state for class 1 apn.
 *
 * @param[out] : state for class 1 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class1_state_get(data_buffer_t *Buffer);

/**
 * Enable class 1 apn.
 *
 * @param ignored n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class1_enable(data_buffer_t *ignored);

/**
 * Disable class 1 apn.
 *
 * @param ignored n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class1_disabled(data_buffer_t *ignored);

/**
 * Getting id for class 2 apn.
 *
 * @param[out] Buffer id value for class 2 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class2_id_get(data_buffer_t *Buffer);

/**
 * Getting name for class 2 apn.
 *
 * @param[out] Buffer name value for class 2 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class2_name_get(data_buffer_t *Buffer);

/**
 * Setting name for class 2 apn.
 *
 * @param[in] Buffer new name value
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class2_name_set(data_buffer_t *Buffer);

/**
 * Getting ip for class 2 apn.
 *
 * @param[out] Buffer ip value for class 2 apn
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class2_ip_get(data_buffer_t *Buffer);

/**
 * Setting ip for class 2 apn.
 *
 * @param[in] Buffer new ip value
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class2_ip_set(data_buffer_t *Buffer);

/**
 * Getting state for class 2 apn.
 *
 * @param[out] Buffer - state for class 2 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class2_state_get(data_buffer_t *Buffer);

/**
 * Enable class 2 apn.
 *
 * @param n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class2_enable(data_buffer_t *Buffer);

/**
 * Disable class 2 apn.
 *
 * @param n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class2_disabled(data_buffer_t *Buffer);

/**
 * Getting id for class 3 apn.
 *
 * @param[out] Buffer id value for class 3 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class3_id_get(data_buffer_t *Buffer);

/**
 * Getting name for class 3 apn.
 *
 * @param[out] Buffer name value for class 3 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class3_name_get(data_buffer_t *Buffer);

/**
 * Setting name for class 3 apn.
 *
 * @param[in] Buffer new name value
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class3_name_set(data_buffer_t *Buffer);

/**
 * Getting ip for class 3 apn.
 *
 * @param[out] Buffer ip value for class 3 apn
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class3_ip_get(data_buffer_t *Buffer);

/**
 * Setting ip for class 3 apn.
 *
 * @param[in] Buffer new ip value
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class3_ip_set(data_buffer_t *Buffer);

/**
 * Getting state for class 3 apn.
 *
 * @param[out] Buffer state for class 3 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class3_state_get(data_buffer_t *Buffer);

/**
 * Enable class 3 apn.
 *
 * @param n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class3_enable(data_buffer_t *Buffer);

/**
 * Disable class 3 apn.
 *
 * @param n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class3_disabled(data_buffer_t *Buffer);

/**
 * Getting id for class 4 apn.
 *
 * @param[out] Buffer id value for class 4 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class4_id_get(data_buffer_t *Buffer);

/**
 * Getting name for class 4 apn.
 *
 * @param[out] Buffer name value for class 4 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class4_name_get(data_buffer_t *Buffer);

/**
 * Setting name for class 4 apn.
 *
 * @param[in] Buffer new name value
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class4_name_set(data_buffer_t *Buffer);

/**
 * Getting ip for class 4 apn.
 *
 * @param[out] Buffer ip value for class 4 apn
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class4_ip_get(data_buffer_t *Buffer);

/**
 * Setting ip for class 4 apn.
 *
 * @param[in] Buffer new ip value
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class4_ip_set(data_buffer_t *Buffer);

/**
 * Getting state for class 4 apn.
 *
 * @param[out] Buffer state for class 4 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class4_state_get(data_buffer_t *Buffer);

/**
 * Enable class 4 apn.
 *
 * @param n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class4_enable(data_buffer_t *Buffer);

/**
 * Disable class 4 apn.
 *
 * @param n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class4_disabled(data_buffer_t *Buffer);

/**
 * Getting id for class 6 apn.
 *
 * @param[out] Buffer id value for class 6 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class6_id_get(data_buffer_t *Buffer);

/**
 * Getting name for class 6 apn.
 *
 * @param[out] Buffer name value for class 6 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class6_name_get(data_buffer_t *Buffer);

/**
 * Setting name for class 6 apn.
 *
 * @param[in] Buffer new name value
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class6_name_set(data_buffer_t *Buffer);

/**
 * Getting ip for class 6 apn.
 *
 * @param[out] Buffer ip value for class 6 apn
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class6_ip_get(data_buffer_t *Buffer);

/**
 * Setting ip for class 6 apn.
 *
 * @param[in] Buffer new ip value
 * For Android device it is : n/a
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class6_ip_set(data_buffer_t *Buffer);

/**
 * Getting state for class 6 apn.
 *
 * @param[out] Buffer state for class 6 apn
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class6_state_get(data_buffer_t *Buffer);

/**
 * Enable class 6 apn.
 *
 * @param n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class6_enable(data_buffer_t *Buffer);

/**
 * Disable class 6 apn.
 *
 * @param n/a
 * For Android device it is : database with URI Telephony.Carriers.CONTENT_URI
 * @return RESULT_SUCCESS if success
 *         RESULT_BUFFER_OVERFLOW if input buffer is too short
 *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_apn_class6_disabled(data_buffer_t *Buffer);

/**
 * Gets IMS domain value.
 *
 * @param[out] Buffer ims domain value in Buffer
 * @return RESULT_SUCCESS if success
 *         RESULR_BUFFER_OVERFLOW if input buffer is too short
 *         RESULR_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_network_ims_domain_get(data_buffer_t *Buffer);

/**
 * Checks is the "ims sms over ip network" feature enabled?
 * @param[out] iobuf
 * @parblock
 * iobuf->data contains c-string:
 *   "True" : "ims sms over ip network" is enabled
 *   "False": "ims sms over ip network" is disabled
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_ims_sms_over_ip_network_indication_get(data_buffer_t *iobuf);

/**
 * Enables or disables the "ims sms over ip network" feature
 * @param[in] iobuf
 * @parblock
 * iobuf->data can contain c-string:
 *   "True" : enable "ims sms over ip network"
 *   "False": disable "ims sms over ip network"
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_ims_sms_over_ip_network_indication_set(data_buffer_t *iobuf);

/**
 * Getting the "ims smsformat" value
 * @param[out] Buffer
 * @parblock
 * Buffer->data contains c-string:
 * "3GPP" or "3GPP2"
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_ims_smsformat_get(data_buffer_t *Buffer);

/**
 * Set the "ims smsformat" feature
 * @param[in] Buffer
 * @parblock
 * Buffer->data can contain c-string:
 *  "3GPP" or "3GPP2"
 * @endparblock
 * @return @see enum result_states
 */
int pal_network_ims_smsformat_set(data_buffer_t *Buffer);

#ifdef __cplusplus
}
#endif

#endif // PAL_CONNMO_H
