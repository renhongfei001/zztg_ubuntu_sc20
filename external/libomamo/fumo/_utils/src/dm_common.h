/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef DM_COMMON_H
#define DM_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include "pal_common.h"


#define DM_UNUSED(var) ((void)var)

typedef struct
{
    uint8_t * buffer;
    size_t    len;
} buffer_t;

/// Management object alert structure
typedef struct {
    char *mo;         ///< management object
    char *alert_data; ///< management object alert data
} alert_t;

/// OMADM request structure
typedef struct {
    omadm_request_type_t type; ///< request type
    buffer_t *package0;        ///< data as package0 message
    alert_t *alert;            ///< data as alert message
} omadm_request_t;

/// OMADM data structure for setup server connection
typedef struct
{
    bool format_type;  /// 0 - xml, 1 - wbxml
    char *server_id;   /// server identificator
    int session_id;    /// session identificator
    /// \todo add all required parameters
} omadm_env_t;

#endif // DM_COMMON_H
