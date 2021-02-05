/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef OMADM_CONTROLLER_H
#define OMADM_CONTROLLER_H

#include "internals.h"
#include "omadmclient.h"
#include "pal.h"
#include "dm_common.h"

#define OMADM_ALERT_MORE_MESSAGES "1222"
#define OMADM_ALERT_SESSION_ABORT "1223"

#ifdef ANDROID
/**
 * User Initiated Update Error Condition display type dialog
 * This controller dialog type define should be in sync with definition
 * inside Android OmadmControllerDialog class
 */
#define DMCLT_UI_TYPE_DISPLAY_UIE 6
#endif

/**
 * Load eternal plugins at start up from the plugins directory
 * @brief omadm_controller_load_eternal_plugins
 */
void omadm_controller_load_eternal_plugins();

/**
 * Handler for a new OMADM request
 * @brief Handle of OMADM request
 * @param[in] request the parameters required to start session
 * @return status: DMCLT_ERR_NONE if success
 *                 error status otherwise
 */
dmclt_err_t omadm_controller_handle_request(void * request);

/**
 * Show UI dialog
 * @brief Show UI dialog
 * @param[in] type of UI dialog
 * @return status: DMCLT_ERR_NONE if success
 *                 error status otherwise
 */
dmclt_err_t omadm_controller_show_ui_dialog(dmclt_ui_type_t type);

/**
 * Starts a new OMADM session
 * @brief Start OMADM session
 * @param[in] request the parameters required to start session
 * @return status: DMCLT_ERR_NONE if success
 *                 error status otherwise
 */
dmclt_err_t omadm_controller_start_session(omadm_request_t * omadm_request);

/**
 * Cancels ongoing OMADM session
 * @brief Cancel OMADM session
 */
dmclt_err_t omadm_controller_cancel_session();

/**
 * Setups format type, server Id and session Id for OMADM session
 * @brief Setup OMADM session
 * @return status: DMCLT_ERR_NONE if success
 *                 DMCLT_ERR_INTERNAL otherwise
 */
dmclt_err_t omadm_controller_setup_session(omadm_env_t *env);

#endif // OMADM_CONTROLLER_H
