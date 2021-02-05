/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef OMADM_INTERFACE_HANDLER_H_
#define OMADM_INTERFACE_HANDLER_H_

#include <omadmclient.h>

/**
 * OMADM Interface Handler Module API
 *
 *  Send post request
 *
 * @param[in] packet  syncML buffer for sending to DM Server
 * @param[in] reply   syncML buffer which DM Server is retrieved for DM Client
 * @param[in] format_type  parameter which determine format type wbxml or not
 *                         should be used for build header for post request
 * @param[in] auth_type parameter which determine SyncML authorization type between
 * @param[in] username  username
 * @param[in] password  password
 * @returns DMCLT_ERR_NONE if successful or one of DMCLT_ERR_*
 */
dmclt_err_t omadm_interface_handler_post_package(dmclt_buffer_t* packet,
                                                 dmclt_buffer_t* reply,
                                                 bool format_type,
                                                 char *username,
                                                 char *password);

/**
 * OMADM Interface Handler Module API
 *
 * Destroy connection
 *
 */
void omadm_interface_handler_end_session();

#endif /* OMADM_INTERFACE_HANDLER_H_*/
