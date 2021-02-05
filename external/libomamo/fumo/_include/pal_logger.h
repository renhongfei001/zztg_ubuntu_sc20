/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef PAL_LOGGER_H
#define PAL_LOGGER_H

/*!
 * @file pal_logger.h
 *
 * @brief
 * Macroses and functions for logging
 *
 */
#define LOGTAG "PAL"
#ifndef LOG_STDOUT
#include <syslog.h>
#else
#include <stdio.h>
#endif

/**
 * @enum PalLogPriority
 *
 * @brief Priorities of DM's log messages.
 * NOTE: (currently mapped to syslog priorities)
*/
typedef enum {
    PAL_CRIT = 2,    ///< 2 == <code>LOG_CRIT</code> from "syslog.h"
    PAL_ERR,         ///< 3 == <code>LOG_ERR</code>
    PAL_WARN,        ///< 4 == <code>LOG_WARNING</code>
    PAL_INFO = 6,    ///< 6 == <code>LOG_INFO</code>
    PAL_DEBUG        ///< 7 == <code>LOG_DEBUG</code>
} PalLogPriority;

/// \todo Create logging helpers for different priorities etc.
/// Maybe embed following info to logs:\code __FILE__ || __LINE__ || __func__ \endcode
#ifndef LOG_STDOUT
#define PAL_LOGI(format, ...) syslog(PAL_INFO, format, ## __VA_ARGS__)
#define PAL_LOGE(format, ...) syslog(PAL_ERR, format, ## __VA_ARGS__)
#define PAL_LOGD(format, ...) syslog(PAL_DEBUG, format, ## __VA_ARGS__)
#define PAL_OPEN_LOG(arg) openlog(arg, LOG_PID|LOG_CONS, LOG_DAEMON)
#define PAL_CLOSE_LOG() closelog()
#else
#define PAL_LOGI(format, ...) do { printf("%s_INFO: ", LOGTAG);\
    printf(format, ## __VA_ARGS__);printf("\n");fflush(stdout);\
} while (0)
#define PAL_LOGE(format, ...) do { printf("%s_ERR: ", LOGTAG);\
    printf(format, ## __VA_ARGS__);printf("\n");fflush(stdout);\
} while (0)
#define PAL_LOGD(format, ...) do { printf("%s_DEBUG: ", LOGTAG);\
    printf(format, ## __VA_ARGS__);printf("\n");fflush(stdout);\
} while (0)
#define PAL_OPEN_LOG(arg) do { } while (0)
#define PAL_CLOSE_LOG() do { } while (0)
#endif

#endif // PAL_LOGGER_H
