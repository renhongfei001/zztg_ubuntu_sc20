/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef DM_CONTROLLER_H
#define DM_CONTROLLER_H

/*!
 * @file controller.h
 *
 * @brief Main controller of DMClient
 *
 * It is responsible for handling of the input events queue,
 * initialization of OMA_DM and/or UDM controllers and for dispatching
 * of input events to them.
 */

#include <stdbool.h>

/**
 * @brief Initializes DM Controller and all its components
 * @return true in case of successfull initialization; false - otherwise
 */
bool init_dmcController(void);

/**
 * @brief Deinitializes DM Controller
 */
void stop_dmcController(void);

/**
 * @brief Function to process data roaming event from PAL, controller shall
 *        terminate the omadm session
 */
void dmc_roaming_enabled();

#endif // DM_CONTROLLER_H
