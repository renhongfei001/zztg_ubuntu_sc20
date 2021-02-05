/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef DM_PLUGIN_MANAGER_H
#define DM_PLUGIN_MANAGER_H

/*!
 * @file plugin_manager.h
 *
 * @brief Manager of dynamically loaded MOs
 *
 */

#include <stdbool.h>

/**
 * @brief Initializes DM plugin manager and all its components
 * @return true in case of successfull initialization; false - otherwise
 */
bool init_pluginManager();

/**
 * @brief Deinitializes DM plugin manager
 */
void stop_pluginManager(void);

#endif // DM_PLUGIN_MANAGER_H
