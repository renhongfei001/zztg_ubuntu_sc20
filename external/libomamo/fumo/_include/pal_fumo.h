/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * @file pal_fumo.h
 * @brief File containing PAL interface for FUMO plugin.
 *
 * Firmware Update Management.
 * Firmware Update Management Object enables firmware updates by specifying
 * the locations in the management tree where update packages could be downloaded.
 * It also specifies commands that need to be invoked on specific nodes of the
 * management tree to start an update activity.
 * @see http://openmobilealliance.org/
 */

#ifndef PAL_FUMO_H
#define PAL_FUMO_H

#include <stdbool.h>
#include <stdint.h>

#include "pal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
//                      FIRMWARE UPDATE SECTION                               //
////////////////////////////////////////////////////////////////////////////////

#define PAL_RC_FRMW_UPD_COMPLETED_SUCCESS ((int)0)
#define PAL_RC_FRMW_UPD_COMPLETED_FAILED  ((int)1)
#define PAL_RC_FRMW_UPD_INPROGRESS       ((int)2)
#define PAL_RC_FRMW_UPD_CANCELLED        ((int)3)

/**
 *  @brief Type of callback for progress information
 *  @param[in] context context of firmware update operation
 *  @param[in] progress progress in update operation
 *  @param[in] err_code error code of update operations
        PAL_RC_FRMW_UPD_....
 *  @return HTTP response status code
 *      101 in progress
 *      200 sucess
 *      500 failed
 */
typedef int (*pal_ft_cb_progress)(void *context, unsigned int progress,
                                    int err_code);

/**
 *  @brief Descriptor of firmware update operation
 */
typedef struct pal_update_descriptor_t
{
    char *name; /*!< in, Firmware Package name */
    unsigned long size; /*!< in, Package Size */
    unsigned long required_install_parametr; /*!< in, estimated Update Time */
    char* type; /*!< in, MIME type */
    char* vendor; /*!< in, DM Server Vendor Name */
    char* install_param; /*!< in, Installations parameters */
    pal_ft_cb_progress progress; /*!< in, callback for progress information */
    void *context; /*!< out, context of update operation */
} pal_update_descriptor_t;

/**
 *  @brief it starts firmware update
 *  @param [in, out] ud Firmware update descriptor
 *  @return HTTP response status code
 *      200 sucess
 *      401 bad arguments
 *      418 already started
 *      500 failed
 */
int pal_update_firmware(pal_update_descriptor_t *ud);

/**
 *  @brief it cancels firmware update
 *  @param [in] context context of update operation
 *  @return HTTP response status code
 *      200 sucess
 *      500 failed
 */
int pal_update_firmware_cancel(void *context);

/**
 *  @brief it returns firmware version
 *  @param [out] fmwv firmware version, must be free by "free()"
 *      it is need to have a firmware update package name
 *      like <Manufacturer>_<Model>_<BaseFwVersion>_<TargetFwVersion>
 *      there <TargetFwVersion> is version, it is returned by this function
 *  @return HTTP response status code
 *      200 sucess
 *      500 failed
 */
int pal_get_firmware_version(char **fmwv);

////////////////////////////////////////////////////////////////////////////////
//                      FIRMWARE DOWNLOAD SECTION                             //
////////////////////////////////////////////////////////////////////////////////

#define PAL_RC_FRMW_DOWNLOAD_SUCCESS                        ((int)0)
#define PAL_RC_FRMW_DOWNLOAD_ERROR                          ((int)1)
#define PAL_RC_FRMW_DOWNLOAD_ERROR_NULL                     ((int)2)
#define PAL_RC_FRMW_DOWNLOAD_ERROR_CANNOT_DOWNLOAD          ((int)3)
#define PAL_RC_FRMW_DOWNLOAD_ERROR_CANNOT_PARSE_DESCRIPTOR  ((int)4)
#define PAL_RC_FRMW_DOWNLOAD_ERROR_EMPTY_URL                ((int)5)
#define PAL_RC_FRMW_DOWNLOAD_ERROR_MEMORY                   ((int)6)
#define PAL_RC_FRMW_DOWNLOAD_CANCELLED                      ((int)7)
#define PAL_RC_FRMW_DOWNLOAD_ERROR_VERIFICATION_FAILED      ((int)8)
#define FUMO_RC_PRV_SU_CANCEL                               ((int)9)
#define FUMO_RC_PRV_SU_CANCEL_BEFORE_DEFERRED_EVENT         ((int)10)
#define FUMO_RC_PRV_SU_CANCEL_AFTER_UPDATE                  ((int)11)
#define FUMO_RC_PRV_NOT_AVAILABLE_MEMORY                    ((int)12)
#define PAL_RC_FRMW_DOWNLOAD_SERVER_UNAVAILABLE             ((int)13)
#define PAL_RC_FRMW_DOWNLOAD_PACKAGE_NOT_ACCEPTABLE         ((int)14)
#define PAL_RC_FRMW_INSTALL_CANCELLED                       ((int)15)
#define PAL_RC_FRMW_DOWNLOAD_NO_NETWORK                     ((int)16)
#define PAL_RC_FRMW_DOWNLOAD_WIFI_TIMEOUT                   ((int)17)

/**
 * @brief Description for download process.
 */
typedef struct pal_download_status_t {
    char *serverUrl; /*!< URL\path to OMADM server */
    char *packageName; /*!< file shall be saved with this name */
    void *context; /*!< out, context of update operation */
    int wifi_only; /*!< in, flag if =1 wifi only allowed */
    pal_ft_cb_progress on_progress; /*!< Callback is called for progress
                                        informing */
} pal_download_status_t;

/**
 * @brief Download descriptor
 */
typedef struct pal_download_descriptor_t {
    char *packageName;
    char *ddVersion;
    char *objectUri; //content server URL
    unsigned long size;
    unsigned long requiredInstall;
    char *type;
    char *vendor;
    char *description;
    char *installParam;
    char *preDownloadMessage;
    char *postDownloadMessage;
    char *preDownloadURL;
    char *postDownloadURL;
    char *postUpdateURL;
    char *postUpdateMessage;
} pal_download_descriptor_t;

/**
 * @brief Perform a downloading package from server
 * @param [in] status, Description for download process.
 * @return PAL_RC_FRMW_DOWNLOAD_SUCCESS of success
 *         PAL_RC_FRMW_DOWNLOAD_ERROR if status is NULL
 *         PAL_RC_FRMW_DOWNLOAD_ERROR_NULL if status.serverUrl is NULL
 *         PAL_RC_FRMW_DOWNLOAD_ERROR_CANNOT_DOWNLOAD if cannot download firmware package
 *         PAL_RC_FRMW_DOWNLOAD_ERROR_CANNOT_PARSE_DESCRIPTOR if the information
 *                                                            from manager server is broken
 *         PAL_RC_FRMW_DOWNLOAD_ERROR_MEMORY if cannot start loading of package with update
 */
int pal_download_firmware(pal_download_status_t *status);

/**
 * @brief Cancel download process if exists
 * @param[in] context Context
 * @return PAL_RC_FRMW_DOWNLOAD_SUCCESS if success
 */
int pal_download_cancel(const void *context);

#define DOWNLOAD_DESCRIPTOR_FILE_NAME "download_descriptor.xml"
#define DOWNLOAD_DESCRIPTOR_MAX_FILE_LENGTH 16384

/**
 * Launch a routine with downloading of Download Descriptor
 * @param[in] serverUrl URL of OMADM server
 * @return PAL_RC_FRMW_DOWNLOAD_SUCCESS if success
 *         PAL_RC_FRMW_DOWNLOAD_ERROR_NULL if serverUrl is NULL
 *         PAL_RC_FRMW_DOWNLOAD_ERROR_EMPTY_URL if serverURL is empty
 *         PAL_RC_FRMW_DOWNLOAD_ERROR if request failed
 */
int pal_request_download_descriptor(const char *serverUrl);

/**
 * @brief Fills descriptor structure from a file on local file system
 * @param[in] filename name of file in CWD
 *            if NULL default file name will be used
 * @param[out] out Download Descriptor
 * @return PAL_RC_FRMW_DOWNLOAD_SUCCESS of OK
 */
int pal_create_download_descriptor(const char *fileanme,
                                   pal_download_descriptor_t **dd);

/**
 * @brief Free memory which was allocated for structure pal_download_status_t
 */
void free_download_status_t(pal_download_status_t **ds);

/**
 * @brief Free memory which was allocated for structure pal_download_descriptor_t
 */
void free_download_descriptor_t(pal_download_descriptor_t **dd);

////////////////////////////////////////////////////////////////////////////////
//                  OMADM FUMO PLUGIN INTERFACE                               //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Message type: information presentation for user.
 */
typedef enum {
    emt_pop_up,
    emt_notification
} pal_fumo_gui_message_type;

/**
 * @brief Blocking type of user's dialog: Normal as non blocked,
 * Persistent as blocked (modal dialog).
 */
typedef enum {
    emm_normal,
    emm_persistent
} pal_fumo_gui_message_mode;

/**
 * @brief  Icon's type in user's dialog.
 */
typedef enum {
    eit_none,
    eit_device
} pal_fumo_gui_icon_type;

/**
 * @brief Buttons type in user's dialog.
 */
#define ebt_ok                       0x0001
#define ebt_cancel                   0x0002
#define ebt_yes                      0x0004
#define ebt_no                       0x0008
#define ebt_ignore                   0x0010
#define ebt_later                    0x0020
#define ebt_check_for_update         0x0040
#define ebt_back                     0x0100
#define ebt_home                     0x0200
/* special processing is needed */
#define ebt_choose_files_to_delete   0x0040
#define ebt_close_window             0x0080
#define ebt_check_box_use_wi_fi      0x0100
#define ebt_oom_notification         0x2000
#define ebt_user_defined2            0x4000
#define ebt_user_defined3            0x8000

/**
 * @brief parameter for user reaction omadm callback
 * function.
 */
typedef struct {
    /**
     * @breif fumo plugun state.
     * [in]
     */
    int state;
    /**
     * @breif defered time for start.
     * update process
     * [in]
     */
    int64_t defered_update_time;
    /**
     * @breif flag which determine that update
     * process should be start only if wifi
     * enabled and connected.
     * [in]
     */
    bool wifi_requred;
    /**
     * @breif flag which determine that update
     * process should be start whithout any user reaction
     * [in]
     */
    bool automatic_update_enable;
    /**
     * @breif gui button identificator
     * which was pressed by user
     * [in]
     */
    unsigned int button_id;
} pal_fumo_gui_user_reply_param;

/**
 * @breif It is an information for user's dialog manipulating.
 * [in, out]
 */
typedef struct pal_fumo_gui_message_descriptor {
    /*!< fumo state, [in] 0 by default */
    int state;

    /*!< message type: popup, notification, etc.
     *   it is set to emtPopUp by default
     *  [in]
     */
    pal_fumo_gui_message_type message_type;

    /*!< message mode: blocking\free
     *   it is set to emmNormal by default
     *  [in]
     */
    pal_fumo_gui_message_mode message_mode;

    /*!< icon in dialog it is set to eitDevice by default
     *  [in]
     */
    pal_fumo_gui_icon_type icon_type;

    /*!< header's text. If headerText_ is NULL message
     *   will not have a hader text.
     *   it is set to 0 by default
     *  [in]
     */
    char *header_text;

    /*!< message's text. If messageText_ is NULL
     *   message will not have a main text.
     *   it is set to 0 by default
     *  [in]
     */
    char *message_text;

    /*!< hyperlink's caption.
     *   If hyperLinkCaption_ is NULL caption will be same as hyperlink.
     *   it is set to 0 by default
     *  [in]
     */
    char *hyper_link_caption;

    /*!< URI\URL. If hyperLink_ is NULL message will not
     *   have a hyperlink and hyperLinkCaption_ must ne ignored.
     *   it is set to 0 by default
     *  [in]
     */
    char *hyper_link;

    /*!< caption of progress bar element.
     *   If progressBarCaption_ is NULL message will not have
     *   a progress bar
     *   it is set to 0 by default
     *  [in]
     */
    char *progress_bar_caption;

    /*!< buttons types: ebtOK & ebtCancel & ebtIgnore, for example
     *   it is set to ebtOK by default
     *  [in]
     */
    unsigned int button_type;

    /*!< time which takes time loading/updating
     */
    char *install_param;

    /*!< available space in bytes for executing system update
     */
    uint64_t required_space_for_update;

    /*!< space in bytes which it is needed to delete
     *   for executing system update
     */
    uint64_t required_space_for_delete;

    /*!< optional/mandatory
     */
    char *severity;

    /*!< callback with codes of pressed\selected buttons
     *   it is set to NULL by default
     *  [in]
     */
    omadmCallback user_reaction;
} pal_fumo_gui_message_descriptor;

/**
 * @breif It creates a dialog for interaction with user.
 * @param [in] pmd, it is an information for user's dialog manipulating
 * @return On success, it returns 0; on error, it returns an error number.
 */
int pal_fumo_gui_create_message(pal_fumo_gui_message_descriptor* pmd);

/**
 * @breif It remove user's dialog.
 * @param [in] state unique value for message's identifing
 * @return On success, it returns 0; on error, it returns an error number.
 */
int pal_fumo_gui_destroy_message(int32_t state);

/**
 * Sets fumo plugin download progress.
 *
 * @param[in] percent current percent of download package
 * @return RESULT_SUCCESS if success
 */
int pal_fumo_gui_update_download_progress(int32_t percent);

/**
 * @breif It is an information about latest system update.
 * [in]
 */
typedef struct pal_fumo_gui_system_update_info {
    /**
     * @breif device software version
     * [in]
     */
    char *software_verion;
    /**
     * @breif device configuration version
     * [in]
     */
    char *configuration_version;
    /**
     * @breif time when we had latest system update
     * [in]
     */
    int64_t latest_system_update_time;
    /**
     * @breif status message about process
     * of current system update
     * [in]
     */
    char *status_message;
    /**
     * @breif url deals with current system update
     * if update was aplplied but no more then 30 day ago
     * then url should be avaliable to check
     * [in]
     */
    char *hyper_link;
} pal_fumo_gui_system_update_info;

/**
 * Send system update info from fumo plugin.
 *
 * @param [in] psu information about system update
 * @return On success, it returns 0; on error, it returns an error number.
 */
int pal_fumo_gui_set_system_update_info(pal_fumo_gui_system_update_info* psu);


////////////////////////////////////////////////////////////////////////////////
//                   GENERAL FUCTIONALITY SECTION                             //
////////////////////////////////////////////////////////////////////////////////
/**
 *  @brief Max command length
 */
#define MAXCMDLEN ((int)1024)
/**
 *  @brief The size of a file wich will be downloaded (in bytes)
 */
#define FUMO_DOWNLOAD_FILE_SIZE ((unsigned long int)100 * 1024 *  1024)
/**
 *  @brief The maximum package size
 */
#define FUMO_MAX_PKG_SIZE ((unsigned long)2147483647)
/**
 *  @brief The minimum level of the battery which is required for updating
 */
#define FUMO_BATTERY_LEVEL ((float)40)


/**
 * @brief it is called by initFunc
 * @return On success, it returns 0
 */
int pal_fumo_init(void);

/**
 * @brief it is called by closeFunc
 * @return On success, it returns 0
 */
int pal_fumo_close(void);

/**
 *  @brief Checks available space on the device
 *  @param[in] need_space Size (in bytes) of the file which
 *  we will download
 *  @return MO_ERROR_SUCCESS if success
 */
int pal_fumo_check_avail_space(unsigned long int need_space);

/**
 *  @brief it returns required size for download or update
 *  @param [out] download_size, required size for download
 *  @param [out] install_size, required size for update
 *  @param [in] pkg_url path to package
 *  @return RESULT_SUCCESS if success
 */
int pal_fumo_get_required_size (long unsigned* download_size,
    unsigned long* install_size, char *pkg_url);

/**
 *  @brief notification about network changing
 *  @param [in] nc network conditions
 *  @return MO_ERROR_SUCCESS if success
 */
int pal_fumo_network_conditions_changed(network_conditions_t nc);

/**
 *  @brief check device enterprise policy
 *  @param [in] package name
 *  @param [in] update path
 *  @return MO_ERROR_SUCCESS if success
 */
int pal_policy_check(char * pkg_name, char * pkg_path);

/**
 * Truncate last 4 bytes of checksum of package
 * @param filename name of file
 * @return 0 if OK
 *         1 if error
 */
int pal_truncate_checksum(char *filename);

/**
 * Verify CRC32 of a file
 * @param file_path full path to the file
 * @return MO_ERROR_SUCCESS if OK
 *         MO_ERROR_COMMAND_FAILED if error
 */
unsigned int pal_verify_Crc32 (char* file_path);
#ifdef __cplusplus
}
#endif

#endif // PAL_FUMO_H
