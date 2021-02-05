/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net_manager.h"
#include "dm_logger.h"

bool netm_is_wifi_connected()
{
    DM_LOGI("MOCKUP: netm_is_wifi_connected - true");
    return true;
}

void netm_up_admin_network()
{
    DM_LOGI("MOCKUP: netm_up_admin_network");
}

void netm_down_admin_network()
{
    DM_LOGI("MOCKUP: netm_down_admin_network");
}

bool netm_is_network_ready()
{
    DM_LOGI("MOCKUP: netm_is_network_ready - true");
    return true;
}

bool netm_is_network_ready_lock()
{
    DM_LOGI("MOCKUP: netm_is_network_ready_lock - true");
    return true;
}
