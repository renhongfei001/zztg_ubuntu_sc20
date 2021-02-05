/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef _FUMO_H_
#define _FUMO_H_

#include "fumo_error.h"
#include "mo_omadmtree.h"
#include "mo_error.h"
#include "pal.h"
#include "dm_logger.h"

#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#define MO_WORK_PATH "/data/vendor/verizon/dmclient"

#define PAL_INSTALL_DIR "/system/lib"

#define PAL_LIB_NAME "libpal.so"

#define CURRENT_DATA_LOCATION "/data/vendor/verizon/dmclient/data"

#ifndef MO_WORK_PATH
#    error "ERROR: MO_WORK_PATH is not defined"
#endif /*MO_WORK_PATH*/

/**
 *  @brief FUMO_STATE_DIR data's directory
 */
#define FUMO_STATE_DIR "/data/"
/**
    @brief FUMO_STATE_FILE, FUMO state storage
*/
#define FUMO_STATE_FILE "fumo.state.store"
/**
    @brief FUMO_BASE_URI, base uri for FUMO MO
*/
#define FUMO_BASE_URI "./ManagedObjects/FUMO"

/**
    @brief FUMO_URI_CANCEL, CANCEL node
*/
#define FUMO_URI_CANCEL "Cancel"
/**
    @brief FUMO_URI_PKGNAME, relative path for PkgName leaf
*/
#define FUMO_URI_PKGNAME "PkgName"
/**
    @brief FUMO_URI_PKGVERSION, relative path for PkgVersion leaf
*/
#define FUMO_URI_PKGVERSION "PkgVersion"
/**
    @brief FUMO_URI_DOWNLOAD, relative path for Download interior node
*/
#define FUMO_URI_DOWNLOAD "Download"
/**
    @brief FUMO_URI_PKGURL, PkgURL leaf
*/
#define FUMO_URI_PKGURL "PkgURL"
/**
    @brief FUMO_URI_DOWNLOAD_PKGURL, relative path for Download/PkgURL leaf
*/
#define FUMO_URI_DOWNLOAD_PKGURL FUMO_URI_DOWNLOAD "/" FUMO_URI_PKGURL
/**
    @brief FUMO_URI_DOWNLOAD_CANCEL, relative path for Download/Cancel leaf
*/
#define FUMO_URI_DOWNLOAD_CANCEL FUMO_URI_DOWNLOAD "/" FUMO_URI_CANCEL
/**
    @brief FUMO_URI_UPDATE, relative path for Update interior node
*/
#define FUMO_URI_UPDATE "Update"
/**
    @brief FUMO_URI_PKGDATA, PkgData leaf
*/
#define FUMO_URI_PKGDATA "PkgData"
/**
    @brief FUMO_URI_UPDATE_PKGDATA, relative path for Update/PkgData leaf
*/
#define FUMO_URI_UPDATE_PKGDATA FUMO_URI_UPDATE "/" FUMO_URI_PKGDATA
/**
    @brief FUMO_URI_UPDATE_CANCEL, relative path for Update/Cancel leaf
*/
#define FUMO_URI_UPDATE_CANCEL FUMO_URI_UPDATE "/" FUMO_URI_CANCEL
/**
    @brief FUMO_URI_DOWNLOAD_AND_UPDATE, relative path for DownloadAndUpdate
           interior node
*/
#define FUMO_URI_DOWNLOAD_AND_UPDATE "DownloadAndUpdate"
/**
    @brief FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL, relative path for
           DownloadAndUpdate/PkgURL leaf
*/
#define FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL \
            FUMO_URI_DOWNLOAD_AND_UPDATE "/" FUMO_URI_PKGURL
/**
    @brief  FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL, relative path for
            DownloadAndUpdate/Cancel leaf
*/
#define FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL \
    FUMO_URI_DOWNLOAD_AND_UPDATE "/" FUMO_URI_CANCEL
/**
    @brief FUMO_URI_STATE, relative path for State leaf
*/
#define FUMO_URI_STATE "State"
/**
    @brief FUMO_URI_EXT, relative path for Ext interior node
*/
#define FUMO_URI_EXT "Ext"
/**
    @brief FUMO_URI_EXT_SEVERITY, relative path for Ext/Severity leaf
*/
#define FUMO_URI_EXT_SEVERITY "Ext/Severity"
/**
    @brief FUMO_URI_EXT_NI, relative path for Ext/NI interior node
*/
#define FUMO_URI_EXT_NI "Ext/NI"
/**
    @brief FUMO_URI_EXT_NI_WIFIONLY, relative path for Ext/NI/WifiOnly leaf
*/
#define FUMO_URI_EXT_NI_WIFIONLY "Ext/NI/WifiOnly"
/**
    @brief FUMO_URI_EXT_NI_WIFIONLYTIMER, relative path for
           Ext/NI/WifiOnlyTimer leaf
*/
#define FUMO_URI_EXT_NI_WIFIONLYTIMER "Ext/NI/WifiOnlyTimer"
/**
    @brief FUMO_URN, FUMO Management Object identifier
*/
#define FUMO_URN "urn:oma:mo:oma-fumo:1.0"

/**
    @brief Idle / Start No pending operation
*/
#define FUMO_DEVICE_STATE_IDLE                  ((int)10)
/**
    @brief Download failed
*/
#define FUMO_DEVICE_STATE_DOWNLOAD_FAILED       ((int)20)
/**
    @brief Download has started
*/
#define FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING  ((int)30)
/**
    @brief Download has been completed successfully
*/
#define FUMO_DEVICE_STATE_DOWNLOAD_COMPLETE     ((int)40)
/**
    @brief Have data and awaiting command to start update
*/
#define FUMO_DEVICE_STATE_READY_TO_UPDATE       ((int)50)
/**
    @brief Update has started
*/
#define FUMO_DEVICE_STATE_UPDATE_PROGRESSING    ((int)60)
/**
    @brief Update failed but have update package
*/
#define FUMO_DEVICE_STATE_UPDATE_FAILED         ((int)70)
/**
    @brief Update failed and no update package available
*/
#define FUMO_DEVICE_STATE_UPDATE_FAILED_NO_DATA ((int)80)
/**
    @brief Update complete and data still available
*/
#define FUMO_DEVICE_STATE_UPDATE_COMPLETE_DATA  ((int)90)
/**
    @brief Data deleted or removed after a successful Update
*/
#define FUMO_DEVICE_STATE_UPDATE_COMPLETE_NO_DATA ((int)100)

/** Additional UI states */

/**
    @brief Some operation failed because of OOM condition
*/
#define FUMO_DEVICE_STATE_MEMORY_NOT_ENOUGH ((int)110)

/**
    @brief Update package is available for downloading
*/
#define FUMO_DEVICE_STATE_UPDATE_AVALIABLE ((int)120)

/**
    @brief Update impossible because of Low Battery condition
*/
#define FUMO_DEVICE_STATE_LOW_BATTERY ((int)130)

/**
    @brief Download over WiFi selected
*/
#define FUMO_DEVICE_STATE_DOWNLOAD_OVER_WIFI ((int)140)

/**
    @brief Download over WiFi, WiFi is not connected
*/
#define FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED ((int)150)

/**
    @brief Wait for battery state above 80%
*/
#define FUMO_DEVICE_STATE_BATTERY_WAITING ((int)160)

/**
    @brief Download deferred
*/
#define FUMO_DEVICE_STATE_DOWNLOAD_DEFERRED ((int)170)

/**
    @brief Update deferred
*/
#define FUMO_DEVICE_STATE_UPDATE_DEFERRED ((int)180)

#define FUMO_DEVICE_STATE_UPDATE_RESTART ((int)190)


#define __SHORT_FILE__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define DBG(__FMT__, ...)              \
    DM_LOGD("%15s:%4d:%15s: " __FMT__, \
        __SHORT_FILE__, __LINE__,      \
        __FUNCTION__, ##__VA_ARGS__);  \

#define WRN(__FMT__, ...)              \
    DM_LOGW("%15s:%4d:%15s: " __FMT__, \
        __SHORT_FILE__, __LINE__,      \
        __FUNCTION__, ##__VA_ARGS__);  \

#define ERR(__FMT__, ...)              \
    DM_LOGE("%15s:%4d:%15s: " __FMT__, \
        __SHORT_FILE__, __LINE__,      \
        __FUNCTION__, ##__VA_ARGS__);  \

#define BZEROTYPE(__VALUE__) bzero((void*)(&(__VALUE__)), sizeof(__VALUE__))

#define FREE_IF(__POINTER__)     \
    if (NULL != (__POINTER__)) { \
      free(__POINTER__);         \
      (__POINTER__) = NULL;      \
    }

#define STRLEN_OR_NULL(__PSTR__) \
    (NULL == (__PSTR__) ? strlen(__PSTR__) : strlen("NULL"))

#define STR_OR_NULL(__PSTR__) \
    (NULL == (__PSTR__) ? (__PSTR__) : "NULL")

/* @brief mutex for wait Wi-Fi connected */
extern pthread_mutex_t wifi_wait_mutex;

/* @brief conditional variable for wait Wi-Fi connected */
extern pthread_cond_t wifi_wait_conditional;

/* @brief wifi_only_timer_expired_flag WiFiOnlyTimer expired */
extern int wifi_only_timer_expired_flag;

/* @brief mutex for retry timeout */
extern pthread_mutex_t retry_timeout_mutex;

/* @brief conditional variable for wait retry timeout */
extern pthread_cond_t retry_timeout;

/*!
 * @brief Callback to initialize the MO (MANDATORY)
 *
 * @param [out] dataP opaque pointer to MO internal data
 *
 * @returns a MO error code
 */
int fumo_root_init_fn(void **dataP);
/*!
 * @brief Callback to free the MO
 *
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 */
void fumo_root_close_fn(void *data);
/*!
 * @brief Callback to get the type of a node
 *
 * @param [in] uri URL of the node
 * @param [out] type type of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_is_node_fn(const char *uri, omadmtree_node_kind_t *type,
                                void *data);
/*!
 * @brief Callback to find the URLs associated to an URN
 *
 * @param [in] urn URN to find
 * @param [out] urlsP null-terminated array of urls, freed by the caller
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_find_urn_fn (const char * urn, char *** urlsP, void * data);
/*!
 * @brief Callback to get the value of a node
 *
 * result is stored in the nodeP parameter. If the targeted node is an
 * interior node, the nodeP->data_buffer must be a char * containing
 * the node's children's names separated by '/'.
 *
 * @param [in, out] nodeP the node to retrieve
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_get_fn (dmtree_node_t * nodeP, void * data);
/*!
 * @brief Callback to set the value of a node
 *
 * The targeted node can already exist. This is used both for ADD
 * and REPLACE MO commands.
 * If nodeP-> type is "node", an interior node must be created.
 *
 * @param [in] nodeP the node to store
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_set_fn (const dmtree_node_t * nodeP, void * data);
/*!
 * @brief Callback to get the ACL of a node
 *
 * The ACL string must be allocated by this function. It will be
 * freed by the caller.
 * If the node has no ACL, *aclP must be NULL.
 *
 * @param [in] uri URL of the node
 * @param [out] aclP ACL of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_get_acl_fn (const char * uri, char ** aclP, void * data);
/*!
 * @brief Callback to set the ACL of a node
 *
 * @param [in] uri URL of the node
 * @param [in] acl ACL of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_set_acl_fn(const char * uri, const char *acl, void * data);
/*!
 * @brief Callback to execute the function associated to a node
 *
 * @param [in] uri URL of the node
 * @param [in] cmdData parameter past to the EXEC MO command
 * @param [in] correlator correlator associated to the EXEC MO command
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int fumo_root_exec_fn (const char * uri, const char * cmdData,
                             const char * correlator, void * data);






// forward declaration for tfnNodeCommand function pointer typedef
typedef struct fumo_node fumo_node;

/**
    @brief execute "Exec" command
    @param [in] pNode, node for execution
    @param [in] cmdData, parameter past to the EXEC SyncML command
    @param [in] correlator, correlator associated to the EXEC SyncML command
    @return FUMO_RC_ACCEPTED_FOR_PROCESSING if success
*/
int node_command_exec(fumo_node* pNode,
                      const char * cmdData, const char * correlator);
/**
    @brief node format in enum
*/
typedef enum fumo_node_format {
    efnf_Unknown = 0,
    efnf_Node,
    efnf_Char,
    efnf_Bin,
    efnf_Int,
    efnf_Bool
} fumo_node_format;

/**
    @brief  system update phases
*/
typedef enum fumo_process_type {
    DOWNLOAD = 0,
    UPDATE
} fumo_process_type;

/**
    @brief convering fumo_node_format to string of format
    @param [in] nf, node's format as type
    @param [out] format, node's format, shall be freeded by "free"
    @return !0 if memory isn't enough
*/
int get_string_node_format(fumo_node_format nf, char **format);
/**
    @brief convering fumo_node_format to string of type
    @param [in] nf, node's format as type
    @param [out] format, node's format, shall be freeded by "free"
    @return !0 if memory isn't enough
*/
int get_string_node_type(fumo_node_format nf, char **type);

/**
    @brief node's value
*/
typedef struct fumo_node_value {
    char* data_;        /*!< data as chars array, must be "free()" */
    unsigned int size_; /*!< size of data in bytes */
} fumo_node_value;

/**
    @brief It returns node's value as bytes array, accrording to
            node format
    @param [in] n, node object
    @param [out] size, data size
    @param [out] data, data as bytes array
    @return MO_ERROR_NONE if success
*/
int get_node_value(const fumo_node* n, unsigned int* size, char** data);
/**
    @brief It sets node's value as bytes array, accrording to
            node format
    @param [in] n, node object
    @param [in] size, data size
    @param [in] data, data as bytes array
    @return MO_ERROR_NONE if success
*/
int set_node_value(fumo_node* n, const unsigned int size, const char* data);

/**
    @breif FUMO_NODE_ABILITY_NONE, Node's posibility to do a nothing
*/
#define FUMO_NODE_ABILITY_NONE      ((unsigned int)0x0000)
/**
    @breif FUMO_NODE_ABILITY_EXEC, Node's posibility to do a command "Exec"
*/
#define FUMO_NODE_ABILITY_EXEC      ((unsigned int)0x0001)
/**
    @breif FUMO_NODE_ABILITY_GET, Node's posibility to do a command "Get"
*/
#define FUMO_NODE_ABILITY_GET       ((unsigned int)0x0002)
/**
    @breif FUMO_NODE_ABILITY_REPLACE, Node's posibility to do
        a command "Replace"
*/
#define FUMO_NODE_ABILITY_REPLACE   ((unsigned int)0x0004)

/**
    @brief struct fumo_node, node's description
*/
typedef struct fumo_node {
    char *uri_;                         /*!< URI */
    char *urn_;                         /*!< URN */
    omadmtree_node_kind_t node_kind_;   /*!< node type */
    fumo_node_format format_;           /*!< node format */
    fumo_node_value value_;             /*!< node's value */
    unsigned int node_command_ability_; /*!< node's posibility to do a
                                             commands */
} fumo_node;

/**
    @brief NODES_COUNT, maximum count of nodes
*/
#define NODES_COUNT ((int)19)
/**
    @brief struct fumo_static_nodes, storage for static nodes
*/
typedef struct fumo_static_nodes {
    fumo_node nodes_[NODES_COUNT];  /*!< array of static nodes */
} fumo_static_nodes;

/**
    param [in, out] g_fumo_static_nodes, description of static nodes
*/
extern fumo_static_nodes g_fumo_static_nodes;

typedef struct fumo_node_offset {
    char* name;          /*!< name as chars array, must be "free()" */
    size_t offset;       /*!< offset of node in storage */
} fumo_node_offset;

typedef struct fumo_nodes_offset {
    fumo_node_offset node[NODES_COUNT];
} fumo_nodes_offset;

/**
    @brief find_node, find node by URI in static part and "dn"
           as dynamic part
    @param [in] uri, URI of searching node
    @param [out] node, found node
    @return MO_ERROR_NONE if success
*/
int find_node(const char *uri, fumo_node **node);

/**
    @brief event for work thread
*/
typedef enum {
    fte_Download,
    fte_Update,
    fte_DownloadAndUpdate,
    fte_Cancel
} fumo_thread_event;

/**
 * @brief Severity values
 */
typedef enum {
    severity_mandatory = 2,
    severity_optional  = 3
} fumo_severity;

/**
    @brief structure for passing arguments to work thread
*/
typedef struct fumo_thread_args {
    fumo_thread_event event_;   /*!< event for work thread */
    fumo_node* node_;           /*!< executed node */
    const char * cmd_data_;     /*!< parameter past to the EXEC SyncML command*/
    const char * correlator_;   /*!< correlator associated to the
                                        EXEC SyncML command */
} fumo_thread_args;

/**
    @param [in, out] g_fumo_work_thread_id, thread ID of
        fumo_thread_Download
*/
extern pthread_t g_fumo_work_thread_id;
extern int g_key_deferred;
/**
    @brief entry point for downolad&update thread
    @param [in] args, arguments, fumo_threadArgs actually
    @return 0 if success
*/
void* fumo_work_thread(void* args);

/**
    @brief Gets WiFi status of the device
    @param [out] WiFi status of the device
        (1 - WiFi is Enabled, 0 - WiFi is Disabled)
    @return error code
*/
typedef int (*ft_wifi_state_get)(int * status);

/**
  * @brief Gets the detailed state of the supplicant's negotiation with an access point.
  * @param[out] outdata
  * @parblock
  *  state of WiFi adapter, possible values:
  *
  *           0 ASSOCIATED
  *           1 ASSOCIATING
  *           2 AUTHENTICATING
  *           3 COMPLETED
  *           4 DISCONNECTED
  *           5 DORMANT
  *           6 FOUR_WAY_HANDSHAKE
  *           7 GROUP_HANDSHAKE
  *           8 INACTIVE
  *           9 INTERFACE_DISABLED
  *           10 INVALID
  *           11 SCANNING
  *           12 UNINITIALIZED
  * @endparblock
  * @return an error code @see enum result_states
  */
typedef int (*ft_network_wifi_status_get)(int *outdata);

/**
    @brief Reads free space on the storage
    @paran [in] index number of mounted storage
    @param [out] fsinfo information about free space on the file system
    @return error code
*/
typedef int (*ft_storage_avail_get)(int index,  fsinfo_t* fsinfo);

/**
    @brief Reads battery level
    @param[in] index number of battery
    @param[out] battery level in can be from 0.0f to 100.0f
    @return error code
*/
typedef int (*ft_battery_level_get)(int index, float* level);

/**
    @brief Reads count of batteries
    @param[out] p_value number of batteries
    @return error code
*/
typedef int (*ft_battery_count_get)(int32_t* p_value);

/**
 *  @brief get network type
 *  @param [out] network type
 *  @return error code
 */
typedef int (*ft_network_type)(int32_t* p_value);

/**
 *  @brief check device enterprise policy
 *  @param [in] package name
 *  @param [in] update path
 *  @return error code
 */
typedef int (*ft_policy_check)(char* name, char* path);
//////////////////////// FUMO STORAGE API /////////////////////////////////////
/**
 *  @brief lenght of pre\post download message
 */
#define DOWNLOAD_MSG_LEN    ((int)1024)
/**
    @brief FUMO storage
    @todo stored in mmap file
*/
typedef struct fumo_storage_t
{
    int state;                              /*!< state */
    int wifi_only;                          /*!< WiFi only flag */
    int wifi_only_timer;                    /*!< WiFi only timer in hours */
    __time_t wifi_only_check_timer;           /*!< WiFi time for check */
    int restart_download;                   /*!< restart download, 0 - No, 1 - Yes*/
    int retry_count;                        /*!< download retry count*/
    char severity[NAME_MAX];                /*!< severity */
    char active_node[NAME_MAX];             /*!< active node path */
    char command_data[NAME_MAX];            /*!< data for EXEC command */
    char correlator[NAME_MAX];              /*!< correlator for alarm */

    int automatic_update_enable;            /*!< automatic_update_enable */
    unsigned long defer_time;               /*!< defer_time */

    /* update descriptor fields */
    unsigned long size;                     /*!< package size */
    unsigned long required_install_parametr;/*!< required space for installation */
    unsigned long required_space_for_update;/*!< Requred available space in bytes for system update */
    unsigned long required_space_for_delete;/*!< Requred delete space in bytes for system update */
    char type[NAME_MAX];                    /*!< MIME type */
    char vendor[NAME_MAX];                  /*!< DM Server Vendor Name */
    char install_param[PATH_MAX];           /*!< installations parameters */
    void *context;                          /*!< context of update operation */

    /* download descriptor fields  */
    char pkg_name[NAME_MAX];                /*!< update package name */
    char pkg_version[NAME_MAX];             /*!< update package version */
    char object_uri[PATH_MAX];              /*!< object uri */
    char description[PATH_MAX];             /*!< description */
    char pre_download_message[DOWNLOAD_MSG_LEN]; /*!< pre download message */
    char post_download_message[DOWNLOAD_MSG_LEN]; /*!< post download message */
    char pre_download_url[PATH_MAX];        /*!< pre download url */
    char post_download_url[PATH_MAX];       /*!< post download url */
    char post_update_url[PATH_MAX];         /*!< post update url */
    char post_update_message[DOWNLOAD_MSG_LEN]; /*!< post update message */

    char download_pkg_url[PATH_MAX];        /*!< download package url */
    char download_update_pkg_url[PATH_MAX]; /*!< download package url */

    char acl_node_root[NAME_MAX];                  /*!< ACL NodeRoot */
    char acl_pkg_name[NAME_MAX];                   /*!< ACL PkgName */
    char acl_pkg_version[NAME_MAX];                /*!< ACL PkgVersion */
    char acl_download[NAME_MAX];                   /*!< ACL Download */
    char acl_download_pkg_url[NAME_MAX];           /*!< ACL DownloadPkgURL */
    char acl_download_cancel[NAME_MAX];            /*!< ACL DownloadCancel */
    char acl_update[NAME_MAX];                     /*!< ACL Update */
    char acl_update_pkg_data[NAME_MAX];            /*!< ACL UpdatePkgData */
    char acl_update_cancel[NAME_MAX];              /*!< ACL UpdateCancel */
    char acl_download_update[NAME_MAX];            /*!< ACL DownloadAndUpdate */
    char acl_download_update_pkg_url[NAME_MAX];    /*!< ACL DownloadAndUpdatePkgURL */
    char acl_download_update_cancel[NAME_MAX];     /*!< ACL DownloadAndUpdateCancel */
    char acl_state[NAME_MAX];                      /*!< ACL State */
    char acl_cancel[NAME_MAX];                     /*!< ACL Cancel */
    char acl_ext[NAME_MAX];                        /*!< ACL Ext */
    char acl_ext_severity[NAME_MAX];               /*!< ACL ExtSeverity */
    char acl_ext_wifi_only[NAME_MAX];              /*!< ACL ExtNIWifiOnly */
    char acl_ext_wifi_only_timer[NAME_MAX];        /*!< ACL ExtNIWifiOnlyTimer */

} fumo_storage_t;

/**
    @brief fumo_storage_open maps fumo storage to memory
    @return MO_ERROR_NONE on success
*/
int fumo_storage_open(void);

/**
    @brief fumo_storage_close unmaps fumo storage to memory
    @return MO_ERROR_NONE on success
*/
int fumo_storage_close(void);

/**
    @brief it returns update package name
    @param [out] pkg_name, update package name, it must be "free()"
    @return MO_ERROR_NONE on success
*/
int fumo_storage_get_pkg_name(char **pkg_name);

/**
    @brief it sets update package name
    @param [in] pkg_name, update package name
    @return MO_ERROR_NONE on success
*/
int fumo_storage_set_pkg_name(char *pkg_name);

/**
    @brief it returns update package version
    @param [out] pkg_version, update package version, it must be "free()"
    @return MO_ERROR_NONE on success
*/
int fumo_storage_get_pkg_version(char **pkg_version);

/**
    @brief it sets update package version
    @param [in] pkg_version, update package version
    @return MO_ERROR_NONE on success
*/
int fumo_storage_set_pkg_version(char *pkg_version);

/**
    @brief it returns update descriptor from storage
    @param [out] update descriptor
    @return MO_ERROR_NONE on success
*/
int fumo_storage_get_update_descriptor(
                                pal_update_descriptor_t *update_descriptor);

/**
 *  @brief it returns state for node "State"
 *  @param [out] state for node "State", it must be one from
 *      FUMO_DEVICE_STATE_* macro
 *  @return MO_ERROR_SUCCESS on success
 */
int fumo_storage_get_state(int *state);

/**
 *  @brief it sets state for node "State"
 *  @param [in]  state for node "State", it must be one from
 *      FUMO_DEVICE_STATE_* macro
 *  @return MO_ERROR_SUCCESS on success
 */
int fumo_storage_set_state(int state);

/* forward declaration */
typedef struct pal_download_descriptor_t pal_download_descriptor_t;
/**
    @brief it sets download desciptor into storage
    @param [in] download desciptor
    @return MO_ERROR_NONE on success
*/
int fumo_storage_set_download_descriptor(
                                pal_download_descriptor_t *download_desciptor);
/*
 *  @brief it sets string value by offset
 *  @param [in] offset for value in fumo_storage_t,
 *      use offset(fumo_storage_t, pkg_name)
 *  @param [in] max_len of value into fumo_storage_t
 *  @param [in] value_len, len of value, excluding terminating '\0'
 *  @param [in] value for set
 *  @return MO_ERROR_NONE on success
 */
int fumo_storage_set_string_value(int offset, size_t max_len,
                                        const char *value, size_t value_len);
/**
 *  @brief it gets string value by offset
 *  @param [in] offset for value in fumo_storage_t,
 *      use offset(fumo_storage_t, pkg_name)
 *  @param [in] max_len of value into fumo_storage_t
 *  @param [out] returned value, must be "free()"
 *  @return MO_ERROR_NONE on success
 */
int fumo_storage_get_string_value(int offset, size_t max_len, char **value);
/*
 *  @brief it sets int value by offset
 *  @param [in] offset for value in fumo_storage_t,
 *      use offset(fumo_storage_t, state)
 *  @param [in] value for set
 *  @return MO_ERROR_NONE on success
 */
int fumo_storage_set_int_value(int offset, int value);
/**
 *  @brief it gets int value by offset
 *  @param [in] offset for value in fumo_storage_t,
 *      use offset(fumo_storage_t, pkg_name)
 *  @param [out] returned value
 *  @return MO_ERROR_NONE on success
 */
int fumo_storage_get_int_value(int offset, int *value);
/*
 *  @brief it sets int value by offset
 *  @param [in] offset for value in fumo_storage_t,
 *      use offset(fumo_storage_t, state)
 *  @param [in] value for set
 *  @return MO_ERROR_NONE on success
 */
int fumo_storage_set_long_value(int offset, unsigned long value);
/**
 *  @brief it gets int value by offset
 *  @param [in] offset for value in fumo_storage_t,
 *      use offset(fumo_storage_t, pkg_name)
 *  @param [out] returned value
 *  @return MO_ERROR_NONE on success
 */
int fumo_storage_get_long_value(int offset, unsigned long *value);
/**
 *  @brief it sets __time_t value by offset
 *  @param [in] offset for value in fumo_storage_t,
 *      use offset(fumo_storage_t, __time_t)
 *  @param [in] value for set
 *  @return MO_ERROR_NONE on success
 */
int fumo_storage_set_time_value(int offset, __time_t value);
/**
 *  @brief it gets __time_t value by offset
 *  @param [in] offset for value in fumo_storage_t,
 *      use offset(fumo_storage_t, __time_t)
 *  @param [out] returned value
 *  @return MO_ERROR_NONE on success
 */
int fumo_storage_get_time_value(int offset, __time_t *value);
/**
 *  @brief reset WiFiOnly and WiFiOnlyTimer for default values
 *  @return MO_ERROR_SUCCESS on success
 */
int fumo_storage_reset_wifi();

/**
 *  @brief reset defer_time and flags for default values
 *  @return MO_ERROR_SUCCESS on success
 */
int fumo_storage_rest_defer_time();

/**
    @brief starts firmware update
    @param [in, out] ud Firmware update descriptor
    @return HTTP response status code
*/
typedef int (*ft_update_firmware)(pal_update_descriptor_t *ud);

/**
    @brief cancels firmware update
    @param [in] context of update operation
    @return HTTP response status code
*/
typedef int (*ft_pal_update_firmware_cancel)(void *context);

/**
    @brief starts firmware download
    @param [in, out] ud Firmware update descriptor
    @return HTTP response status code
*/
typedef int (*ft_download_firmware)(pal_download_status_t *ud);

/**
 @brief Cancel of download.
 @param [in] context, unique identifier of download session.
 @return !0 if error, 0 if success
 */
typedef int (*ft_pal_download_cancel)(const void *context);

/**
 * @brief Fills descriptor structure from a file on local file system
 * @param[in] filename name of file in CWD
 *            if NULL default file name will be used
 * @param[out] out Download Descriptor
 * @return PAL_RC_FRMW_DOWNLOAD_SUCCESS of OK
 */
typedef int (*ft_create_download_descriptor)(const char *fileanme,
                                            pal_download_descriptor_t **dd);

/**
 * Free memory, allocated for pal_download_descriptor_t structure
 * @param dd pointer to structure
 */
typedef void (*ft_free_download_descriptor)(pal_download_descriptor_t **dd);

/**
 * Requests download descriptor
 * @param[out] serverUrl Server url
 * Launch a routine with downloading of Download Descriptor
 * @param[in] serverUrl URL of OMADM server
 * @return PAL_RC_FRMW_DOWNLOAD_SUCCESS if success
 *         PAL_RC_FRMW_DOWNLOAD_ERROR_NULL if serverUrl is NULL
 *         PAL_RC_FRMW_DOWNLOAD_ERROR_EMPTY_URL if serverURL is empty
 *         PAL_RC_FRMW_DOWNLOAD_ERROR if request failed
 */
typedef int (*ft_request_download_descriptor)(const char *serverUrl);

/**
 * Reads current mobile network type to the p_value pointer
 * @see ./ManagedObjects/DiagMon/Network/CurrentNetwork
 * @param[out] p_value
 * @parblock
 * pointer to store current mobile network type:
 *   0 : NETWORK_TYPE_UNKNOWN
 *   1 : NETWORK_TYPE_GPRS
 *   2 : NETWORK_TYPE_EDGE
 *   3 : NETWORK_TYPE_UMTS
 *   4 : NETWORK_TYPE_CDMA
 *   5 : NETWORK_TYPE_EVDO_0
 *   6 : NETWORK_TYPE_EVDO_A
 *   7 : NETWORK_TYPE_1xRTT
 *   8 : NETWORK_TYPE_HSDPA
 *   9 : NETWORK_TYPE_HSUPA
 *  10 : NETWORK_TYPE_HSPA
 *  11 : NETWORK_TYPE_IDEN
 *  12 : NETWORK_TYPE_EVDO_B
 *  13 : NETWORK_TYPE_LTE
 *  14 : NETWORK_TYPE_EHRPD
 *  15 : NETWORK_TYPE_HSPAP
 * @endparblock
 * @return @see enum result_states
 */
typedef int (*ft_get_current_network)(int32_t* p_value);

typedef int (*ft_request_admin_network)(bool enable);

/**
 * @brief it cancels the downloading process
 * @return HTTP response status code
*/
int fumo_download_cancel(void);

/**
 * @brief it cancels the updating process
 * @return HTTP response status code
*/
int fumo_update_cancel(void);

/**
 * @breif It creates a dialog for interaction with user.
 * @param [in] pmd, it is an information for user's dialog manipulating
 * @return On success, it returns 0; on error, it returns an error number.
 */
typedef int (*ft_pal_fumo_gui_create_message)(
                                    pal_fumo_gui_message_descriptor* pmd);

/**
 * @breif It remove user's dialog.
 * @param [in] state, it is an unique value for message's identifing
 * @return On success, it returns 0; on error, it returns an error number.
 */
typedef int (*ft_pal_fumo_gui_destroy_message)(int32_t state);

/**
 * Sets fumo plugin download progress.
 *
 * @param[in] percent current percent of download package
 * @return RESULT_SUCCESS if success
 */
typedef int (*ft_pal_fumo_gui_update_download_progress)(int32_t percent);

/**
 * Send system update info from fumo plugin.
 *
 * @param [in] psu, it is an information about system update
 * @return On success, it returns 0; on error, it returns an error number.
 */
typedef int (*ft_pal_fumo_gui_set_system_update_info)(
                                    pal_fumo_gui_system_update_info* psu);

/**
    @brief g_dmclient_event_handler callback for DM-client event handler
*/
extern omadm_mo_ft_event_handler g_dmclient_event_handler;

/**
 *
 * @param[out] Buffer, allocated buffer for return value.
 * Firmware Version of current device
 * For Android device it is a version of application
 * Get by parcing result of shell command "getProp"
 * or
 * using android.os.Build
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes \code RESULT_SUCCESS \endcode
 *
 */
typedef int (*ft_pal_system_fwv_get)(data_buffer_t *Buffer);

/**
 *  @brief it removes all temporary files and sets state to default
 */
void fumo_cleanup(void);

/**
 *  @brief continue FUMO's job from previous state
 *  @param [in] data MO internal data as created by #omadm_mo_init_fn
 *  @return MO_ERROR_NONE if success
 */
int fumo_continue_job(void * data);

/**
 *  @brief callback from network notification PAL's part
 *  @param [in] network_conditions_type_t - type of event
 *  @param [in] bool - data, true - is available, false - is not available
 *  @return MO_ERROR_NONE if success
 */
int network_conditions_changed(network_conditions_t nc);

/**
 * Set callback function for process cellular network events
 *
 * @param[in] netm_handler_process_event Callback function
 * @return non-negative id of registered callback if success, otherwise error
 * code.
 */
typedef int (*pal_notify_on_network_conditions_changed_t)
                    (network_conditions_changed_t netm_handler_process_event);

/**
    @brief Check status of battery level
    @param[out] battery_status flag for battery status
    @return RESULT_SUCCESS if success
*/
int get_battery_level(int*);

/**
    @brief Register callback function
    @param[in] OmadmCallBack id
    @param[in] callback function
*/
typedef int (*pal_register_callback_t)(int callback_id, void* args);

/**
*  @brief Show dialog "System update via Wi-Fi, is not connected"
*  @return MO_ERROR_SUCCESS if success
*/
int show_gui_system_update_wifi_not_connected();

/**
 *  @brief Hide dialog "System update via Wi-Fi"
 *  @return MO_ERROR_NONE if success
 */
int hide_gui_system_update_wifi_not_connected();

/**
 * @brief Shows dialog System update failed
 * @return MO_ERROR_NONE if success
 */
int show_system_update_failed();

/**

 * @brief Gets pointer for PAL library method
 * @param[in] handle
 * @param[in] method_name - null terminated string containing
 * pal library method name
 * @return function pointer or NULL in case of error
 */
void* get_pal_func(void **pal_handle, const char* method_name);

/**
 * @brief check wifi timer expiration
 * @return expiration state value 0 if timer expired
 * or 1 if not.
 */
int wifi_timer_check();

#endif // _FUMO_H_

