/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * @file pal_dcmo.h
 * @brief File containing PAL interface for DCMO plugin.
 *
 * Device Capabilities Managment.
 * The DCMO plugin specifies the mechanisms required for the remote management of device capabilities.
 * In particular it will address the ability of remote enablement and disablement of device
 * capabilities. The device capability information will be exposed by DCMO to facilitate management
 * of the Device Capabilities.
 * @see http://openmobilealliance.org/
 */

#ifndef PAL_DCMO_H
#define PAL_DCMO_H
#include "pal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Get VoLTE VLT status
 * @param[out] Buffer
 * - VoLTE VLT status
 * - Default = False
 * - Enabled = True, Disabled = False
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_BUFFER_OVERFLOW if input buffer is too short
 * - RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_mobile_vlt_state_get(int * out);

/*!
 * Get VoLTE EAB status
 * @param[in] Buffer
 * - VoLTE EAB status
 * - Default = False
 * - Enabled = True, Disabled = False
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_BUFFER_OVERFLOW if input buffer is too short
 * - RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_mobile_eab_state_get(int * out);

/*!
 * Get VoLTE LVC status
 * @param[in] Buffer
 * - VoLTE LVC status
 * - Default = False
 * - Enabled = True, Disabled = False
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_BUFFER_OVERFLOW if input buffer is too short
 * - RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_mobile_lvc_state_get(int * out);

/*!
 * Get VICE status
 * @param[in] Buffer
 * - VICE status
 * - Default = false
 * - Enabled = True, Disabled = False
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_BUFFER_OVERFLOW if input buffer is too short
 * - RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_mobile_vce_state_get(int * out);

/*!
 * Get VoWiFi status
 * @param[in] Buffer
 * - VoWiFi status
 * - Default = false
 * - Enabled = True, Disabled = False
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_BUFFER_OVERFLOW if input buffer is too short
 * - RESULT_BUFFER_NOT_DEFINED if argument is not defined
 */
int pal_mobile_vwf_state_get(int * out);

/*
 * Enable vlt
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_vlt_enable();

/*
 * Disable vlt
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_vlt_disable();

/*
 * Enable eab
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_eab_enable();


/*
 * Disable eab
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_eab_disable();

/*
 * Enable lvc
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_lvc_enable();

/*
 * Disable lvc
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_lvc_disable();

/*
 * Enable vce
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_vce_enable();

/*
 * Disable vce
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_vce_disable();

/*
 * Enable vwf
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_vwf_enable();

/*
 * Disable vwt
 *
 * @param[in] Buffer
 *
 * - Null
 *
 * @return
 * - RESULT_SUCCESS if success
 * - RESULT_ERROR if not success
 */
int pal_mobile_vwf_disable();

#ifdef __cplusplus
}
#endif

#endif // PAL_DCMO_H
