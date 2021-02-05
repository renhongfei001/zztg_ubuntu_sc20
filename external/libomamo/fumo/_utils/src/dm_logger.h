/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef _DM_LOGGER_H_
#define _DM_LOGGER_H_
/*!
 * @file dm_logger.h
 *
 * @brief
 * Macroses and functions for logging
 *
 */
#ifndef LOGTAG
#define LOGTAG "DM"
#endif
#ifndef LOG_STDOUT
#include <syslog.h>
#else
#include <stdio.h>
#endif
/// \todo Create logging helpers for different priorities etc.
/// Maybe embed following info to logs:\code __FILE__ || __LINE__ || __func__ \endcode
#ifndef LOG_STDOUT
#define DM_LOGI(format, ...) syslog(LOG_INFO, format, ## __VA_ARGS__)
#define DM_LOGE(format, ...) syslog(LOG_ERR, format, ## __VA_ARGS__)
#define DM_LOGD(format, ...) syslog(LOG_DEBUG, format, ## __VA_ARGS__)
#define DM_LOGW(format, ...) syslog(LOG_WARNING, format, ## __VA_ARGS__)

#define DM_OPEN_LOG(arg) openlog(arg, LOG_PID|LOG_CONS, LOG_DAEMON)
#define DM_CLOSE_LOG() closelog()
#else
#define DM_LOGI(format, ...) do { printf("%s_LOGI: ", LOGTAG);\
    printf(format, ## __VA_ARGS__);printf("\n");fflush(stdout);\
} while (0)
#define DM_LOGE(format, ...) do { printf("%s_LOGE: ", LOGTAG);\
    printf(format, ## __VA_ARGS__);printf("\n");fflush(stdout);\
} while (0)
#define DM_LOGD(format, ...) do { printf("%s_LOGD: ", LOGTAG);\
    printf(format, ## __VA_ARGS__);printf("\n");fflush(stdout);\
} while (0)
#define DM_LOGW(format, ...) do { printf("%s_DM_LOGW: ", LOGTAG);\
    printf(format, ## __VA_ARGS__);printf("\n");fflush(stdout);\
} while (0)

#define DM_OPEN_LOG(arg) do { } while (0)
#define DM_CLOSE_LOG() do { } while (0)
#endif
#endif //_DM_LOGGER_H_
