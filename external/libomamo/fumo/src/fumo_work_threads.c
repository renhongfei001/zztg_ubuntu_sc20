/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */
#include <errno.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "fumo.h"
#include "json.h"

#define PALFU_WORK_PATH "./data"

// delay in seconds
#define DELAY_24_HOURS 3600*24
#define DELAY_5_MINUTES 300
#define RECOVERY_SUCCESS_FLAG_FILE_PATH "/cache/update_success_flag"
#define RECOVERY_FAIL_FLAG_FILE_PATH "/cache/update_fail_flag"
#define NOTIFICATION_UI_FLAG_FILE_PATH "/cache/update_flag_complete_ui"
#define DOWNLOAD_FLAG_FILE_PATH "/data/vendor/verizon/dmclient/data/download_flag"
#define NETWORK_UP_FLAG_FILE_PATH "/data/vendor/verizon/dmclient/data/network_up_flag"

static fumo_thread_args *fta = NULL;


/* @brief libpal.so handle for update operation */
void* pal_handle_update = NULL;
/* @brief libpal.so handle for download operation */
void* pal_handle_download = NULL;
/* @brief 1 if UI is supported, 0 for DCD */
extern int gui_supported;
/* @brief mutex for GUI dialogs */
pthread_mutex_t gui_wait_mutex = PTHREAD_MUTEX_INITIALIZER;

/* @brief conditional variable for GUI dialogs */
pthread_cond_t gui_wait_conditional = PTHREAD_COND_INITIALIZER;

/* @brief mutex for wait Wi-Fi connected */
pthread_mutex_t wifi_wait_mutex = PTHREAD_MUTEX_INITIALIZER;

/* @brief conditional variable for wait Wi-Fi connected */
pthread_cond_t wifi_wait_conditional = PTHREAD_COND_INITIALIZER;

/* @brief mutex for retry timeout */
pthread_mutex_t retry_timeout_mutex = PTHREAD_MUTEX_INITIALIZER;

/* @brief conditional variable for retry timeout */
pthread_cond_t retry_timeout = PTHREAD_COND_INITIALIZER;
/* @brief mutex for wait deferred process */
pthread_mutex_t defer_wait_mutex = PTHREAD_MUTEX_INITIALIZER;

/* @brief conditional variable for wait deferred process*/
pthread_cond_t defer_wait_conditional = PTHREAD_COND_INITIALIZER;

/* @brief mutex for downloading */
pthread_mutex_t download_wait_mutex = PTHREAD_MUTEX_INITIALIZER;

/* @brief conditional variable for downloading */
pthread_cond_t download_wait_conditional = PTHREAD_COND_INITIALIZER;

/* @brief mutex for update */
pthread_mutex_t update_wait_mutex = PTHREAD_MUTEX_INITIALIZER;

/* @brief conditional variable for update */
pthread_cond_t update_wait_conditional = PTHREAD_COND_INITIALIZER;

/* @brief parameter for user reaction */
pal_fumo_gui_user_reply_param gui_response;

/* Forward declaration */
int download_on_progess(void *context, unsigned int progress, int err_code);
int check_avail_space(unsigned long);
void on_gui_response(void * data);
void clean_gui_message_descriptor(pal_fumo_gui_message_descriptor * md);
void show_update_error_dialog(char *make, char *model);
void show_update_complete_dialog();
int download_failed_retry(pal_download_status_t *download_status, fumo_thread_args *fta, char* severity);
int wait_wifi();
void check_low_battery_status();
int wifi_only_timer_init();
int wifi_connect_timedwait();

/* @brief download_cancel_flag flag for downloading cancel */
int download_cancel_flag = 0;
/* @brief update_cancel_flag flag for updating cancel */
int update_cancel_flag = 0;

/* @brief download_by_user_cancel_flag flag for download cancel by user*/
int download_by_user_cancel_flag = 0;
/* @brief install_by_user_cancel_flag flag for install cancel by user */
int install_by_user_cancel_flag = 0;
/* @brief continue_download_via_lte continue download via LTE, 0 - No, 1 - Yes */
int continue_download_via_lte = 0;
/* @brief wifi_only_timer_expired_flag WiFiOnlyTimer expired */
int wifi_only_timer_expired_flag = 0;

int g_key_deferred = 0;

int wait_for_user_reaction();
int wait_for_user_reaction_or_timer(int seconds);
static int get_wifi_state();
void remove_fw_package();
/**
 *  @brief alarm type for download process
 */
const char alarm_type_download[] =
    "org.openmobilealliance.dm.firmwareupdate.download";
/**
 *  @brief alarm type for update process
 */
const char alarm_type_update[] =
    "org.openmobilealliance.dm.firmwareupdate.update";
/**
 *  @brief alarm type for downloadandupdate process
 */
const char alarm_type_downloadandupdate[] =
    "org.openmobilealliance.dm.firmwareupdate.downloadandupdate";
/**
    @brief update descriptor
*/
pal_update_descriptor_t g_pal_update_descriptor;

/**
    @brief g_dmclient_event_handler callback for DM-client event handler
*/
omadm_mo_ft_event_handler g_dmclient_event_handler = NULL;

/**
    @brief upate status
*/
int g_update_state = PAL_RC_FRMW_UPD_COMPLETED_FAILED;

/**
    @brief Gets required
    sizes for memory check from
    download descriptor
    @param[out] download_size Size of the downloaded package
    @param[out] install_size Size for installing
    @return RESULT_SUCCESS
*/
int get_required_size (unsigned long*, unsigned long*);

/**
    @brief System Update History, create, update file
    @param[in]
    @return RESULT_SUCCESS
*/
int update_history(int error_code);

/**
 * Wait cancel process or expired timer for deferred process
 * @return id, 0 cancel, -1 expired timer
 */
int wait_defer_timer(unsigned long seconds);

/* Pal functions */
ft_network_wifi_status_get wifi_status_get;
ft_wifi_state_get wifi_state_get;
ft_network_type network_type_get;
ft_storage_avail_get storage_avail_get_func;
ft_battery_level_get battery_level_get_func;
ft_battery_count_get battery_count_get_func;
ft_download_firmware download_firmware_func = NULL;

pal_download_descriptor_t *download_descriptor = NULL;
ft_create_download_descriptor create_download_descriptor_func = NULL;
ft_request_download_descriptor request_download_descriptor_func = NULL;
ft_free_download_descriptor free_download_descriptor_func = NULL;
ft_policy_check pal_fumo_policy_check = NULL;

//ft_update_firmware update_firmware_func;

/**
    @brief to show dialog if wifi_only = false
    @param[in]
    @return RESULT_SUCCESS
*/
int downloading_wifi_only_false(fumo_thread_args *fta, char* severity, char* url);

/**
    @brief cancel timer by press of check for update button
*/
void update_button_wifi_timer_callback();
/**
    @brief cancel defer timer by press of check for update button
*/
void update_button_defer_timer_callback();
/**
    @brief get a result press of check for update button
*/
void get_update_button_result(void* callback);

/**
 * @brief Gets pointer for pal library method
 * @param[in] method_name - null terminated string containing
 * pal library method name
 * @return function pointer or NULL in case of error
 */
void* get_pal_method(const char* method_name)
{
    void* fp = NULL;

    if(NULL == pal_handle_update && NULL == (pal_handle_update=dlopen(
                        PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY))) {
        DM_LOGE("FUMO: Can't open library: %s",
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        return NULL;
    }

    if(NULL == (fp = dlsym(pal_handle_update, method_name))) {
        DM_LOGE("FUMO: Can't get [%s] from %s", method_name,
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        return NULL;
    }
    return fp;
}

/**
 *  @brief it sends alarm to DM-client
 *  @param [in] arguments from work thread
 *  @param [in] http-responce code
 *  @param [in] alert type
 */
void send_alarm_to_client(fumo_thread_args* fta, int alarm,
    const char* alert_type)
{
    const int alarm_data_len = 20;
    omadm_mo_event_t *event = NULL;
    DM_LOGI("FUMO: send alarm to client alarm = %d type = %s",alarm,alert_type);
    if(NULL == g_dmclient_event_handler)
        return;

    if(NULL == (event=(omadm_mo_event_t*)malloc(sizeof(omadm_mo_event_t)))) {
        DM_LOGE("FUMO send_alarm_to_client: cannot allocate omadm_mo_event_t");
        return;
    }

    memset(event, 0, sizeof(omadm_mo_event_t));
    event->correlator = (char*)fta->correlator_;
    if( NULL == (event->dmclt_item.source=strdup(fta->node_->uri_)) ||
        NULL == (event->dmclt_item.target=strdup("com.vzwdmserver")) ||
        NULL == (event->dmclt_item.type=strdup(alert_type)) ||
        0 != get_string_node_format(fta->node_->format_,
            &(event->dmclt_item.format)) ||
        NULL == (event->dmclt_item.data=(char*)malloc(alarm_data_len))
    ) {
        if(event->dmclt_item.source) free(event->dmclt_item.source);
        if(event->dmclt_item.target) free(event->dmclt_item.target);
        if(event->dmclt_item.type) free(event->dmclt_item.type);
        if(event->dmclt_item.format) free(event->dmclt_item.format);
        free(event);
        return;
    }

    snprintf(event->dmclt_item.data, alarm_data_len, "%d", alarm);
    g_dmclient_event_handler(event);
}

/**
 *  @brief it sends su cancel alarm to DM-client and removes update package
 *  @param [in] arguments from work thread
 *  @param [in] http-responce code
 *  @param [in] alert type
 */
void send_su_cancel_alarm_to_client(fumo_thread_args* fta, int alarm,
    const char* alert_type)
{
   send_alarm_to_client(fta, alarm, alert_type);
   fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
   remove_fw_package();
}

/**
    @brief it finds node by URI: uri_root + "/" + uri_search
    @param [in] uri_root first part or URI
    @param [in] uri_search second part or URI
    @return fumo_node* if failed it will return NULL
*/
fumo_node* find_node_by_uri(const char *uri_root, const char *uri_search)
{
    fumo_node* node = NULL;
    int uri_len = 0;
    char* uri_full = NULL;

    uri_len = strlen(uri_root) + strlen("/") + strlen(uri_search) + 1;
    uri_full = (char*)malloc(uri_len);
    if(!uri_full) {
        DM_LOGE("FUMO: find_node_by_uri: Out of memory");
        return NULL;
    }

    strcpy(uri_full, uri_root);
    strcat(uri_full, "/");
    strcat(uri_full, uri_search);

    find_node(uri_full, &node);
    DM_LOGI("FUMO: find_node_by_uri: node = %p", node);
    return node;
};

int update_progress(void *context, unsigned int progress, int err_code)
{
    ft_pal_fumo_gui_update_download_progress update_progress_func = NULL;

    DM_LOGI("FUMO: update_progress, context=%p, progress=%d, err_code=%d",
        context, progress, err_code);

    g_update_state = err_code;

    switch(err_code)
    {
        case PAL_RC_FRMW_UPD_COMPLETED_SUCCESS:
            DM_LOGI("FUMO: update_progress, COMPLETED_SUCCESS");
            break;
        case PAL_RC_FRMW_UPD_COMPLETED_FAILED:
            DM_LOGE("FUMO: update_progress, COMPLETED_FAILED");
            show_update_error_dialog(NULL, NULL);
            break;
        case PAL_RC_FRMW_UPD_CANCELLED:
            DM_LOGI("FUMO: update_progress, UPD_CANCELLED");
            break;
        case PAL_RC_FRMW_UPD_INPROGRESS:
            DM_LOGI("FUMO: update_progress, PAL_RC_FRMW_UPD_INPROGRESS");
            if(gui_supported && NULL != (update_progress_func =
                    (ft_pal_fumo_gui_update_download_progress)get_pal_method(
                    "pal_fumo_gui_update_download_progress"))) {
                update_progress_func(progress);
            }
            break;
        default:
            DM_LOGE("FUMO: update_progress Unknown error code. context=%p, \
progress=%d, err_code=%d", context, progress, err_code);
            return MO_ERROR_INVALID_CREDENTIALS;
    }

    /* cleanup */
    if(PAL_RC_FRMW_UPD_COMPLETED_SUCCESS == err_code ||
                PAL_RC_FRMW_UPD_COMPLETED_FAILED == err_code ||
                PAL_RC_FRMW_UPD_CANCELLED == err_code ) {
        DM_LOGI("FUMO: update_progress: unblock waiting thread");
        pthread_mutex_lock(&update_wait_mutex);
        pthread_cond_signal(&update_wait_conditional);
        pthread_mutex_unlock(&update_wait_mutex);
        DM_LOGI("FUMO: update_progress: cleanup update descriptor");
        free(g_pal_update_descriptor.name);
        free(g_pal_update_descriptor.type);
        free(g_pal_update_descriptor.vendor);
        free(g_pal_update_descriptor.install_param);
        memset(&g_pal_update_descriptor, 0, sizeof(g_pal_update_descriptor));
    }

    return MO_ERROR_NONE;
}


int show_low_batt_notification()
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;
    if (NULL == (gui_create_message_func =
                    get_pal_method("pal_fumo_gui_create_message")))
        return MO_ERROR_COMMAND_FAILED;
    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
                    malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return MO_ERROR_COMMAND_FAILED;
    }
    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->state = FUMO_DEVICE_STATE_LOW_BATTERY;
    message_descriptor->message_type = emt_notification;
    message_descriptor->message_mode = emm_normal;
    message_descriptor->icon_type = eit_device;
    message_descriptor->header_text = strdup("Unable to Apply System Update");
    message_descriptor->message_text = strdup("Low Battery. Charge phone to continue");
    message_descriptor->hyper_link_caption = strdup("Learn More");
    message_descriptor->hyper_link = strdup("http://www.verizonwireless.com/support/");
    message_descriptor->user_reaction = NULL;

    DM_LOGI("FUMO: Show dialog <Unable to Apply System Update - Low Battery>");
    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);
    return MO_ERROR_NONE;
}

/**
 * @brief Shows "System update ready to install" dialog
 * @return 0 if success
 */
int show_system_update_ready_to_install_notification()
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if (NULL == (gui_create_message_func =
                    get_pal_method("pal_fumo_gui_create_message")))
        return 1;

    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
                    malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return 1;
    }

    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->state = FUMO_DEVICE_STATE_DOWNLOAD_COMPLETE;
    message_descriptor->message_type = emt_pop_up;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_device;
    message_descriptor->header_text =
            strdup("System update ready to install");
    if (MO_ERROR_NONE != fumo_storage_get_string_value(
            offsetof(fumo_storage_t, post_download_message),
            PATH_MAX, &(message_descriptor->message_text))) {
        clean_gui_message_descriptor(message_descriptor);
        free(message_descriptor);
        return 1;
    }
    message_descriptor->hyper_link_caption = strdup("Learn More");
    if (MO_ERROR_NONE != fumo_storage_get_string_value(
            offsetof(fumo_storage_t, post_download_url),
            PATH_MAX, &(message_descriptor->hyper_link))) {
        clean_gui_message_descriptor(message_descriptor);
        free(message_descriptor);
        return 1;
    }
    if (MO_ERROR_NONE != fumo_storage_get_string_value(
                offsetof(fumo_storage_t, install_param),
                PATH_MAX, &(message_descriptor->install_param))) {
            clean_gui_message_descriptor(message_descriptor);
            free(message_descriptor);
            return 1;
    }
    if (MO_ERROR_NONE != fumo_storage_get_string_value(
            offsetof(fumo_storage_t, severity),
            NAME_MAX, &(message_descriptor->severity))) {
        clean_gui_message_descriptor(message_descriptor);
        free(message_descriptor);
        return 1;
    }
    message_descriptor->user_reaction = on_gui_response;

    DM_LOGI("FUMO: Show dialog <System update ready to install>");
    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);
    return 0;
}

/**
 * @brief Shows installation progress in the notification tray
 * @return 0 if success
 */
int show_installing_system_update_notification()
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if (NULL == (gui_create_message_func =
                    get_pal_method("pal_fumo_gui_create_message")))
        return 1;

    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
                    malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return 1;
    }

    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->state = FUMO_DEVICE_STATE_UPDATE_PROGRESSING;
    message_descriptor->message_type = emt_notification;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_device;
    message_descriptor->header_text =
            strdup("Installing system update");

    DM_LOGI("FUMO: Show dialog <Installing system update>");
    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);
    return 0;
}

/**
 * @brief Shows dialog System update ready restart
 * @return 0 if success
 */
int show_system_update_ready_restart_notification()
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if (NULL == (gui_create_message_func =
                    get_pal_method("pal_fumo_gui_create_message")))
        return 1;

    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
                    malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return 1;
    }

    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->state = FUMO_DEVICE_STATE_READY_TO_UPDATE;
    message_descriptor->message_type = emt_pop_up;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_device;
    message_descriptor->header_text =
            strdup("System update ready");
    if (MO_ERROR_NONE != fumo_storage_get_string_value(
                offsetof(fumo_storage_t, install_param),
                PATH_MAX, &(message_descriptor->install_param))) {
            clean_gui_message_descriptor(message_descriptor);
            free(message_descriptor);
            return 1;
    }

    message_descriptor->user_reaction = on_gui_response;

    DM_LOGI("FUMO: Show dialog <System update ready>");
    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);
    return 0;
}

/**
 * @brief Shows dialog System update failed
 * @return MO_ERROR_NONE if success
 */
int show_system_update_failed()
{
    DBG("()");
    int rc = MO_ERROR_NONE;
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor message_descriptor;
    BZEROTYPE(message_descriptor);
    pal_int_pbuff_fn pal_system_man_get_fn = NULL;
    pal_int_pbuff_fn pal_system_mod_get_fn = NULL;
    data_buffer_t man_buff;
    data_buffer_t mod_buff;
    BZEROTYPE(man_buff);
    BZEROTYPE(mod_buff);
    pal_system_man_get_fn = (pal_int_pbuff_fn)
      get_pal_method("pal_system_man_get");
    pal_system_mod_get_fn = (pal_int_pbuff_fn)
        get_pal_method("pal_system_mod_get");
    gui_create_message_func = (ft_pal_fumo_gui_create_message)
        get_pal_method("pal_fumo_gui_create_message");
    /*
     * we could check gui_create_message_func here and return from
     * this function immediately, but it can reduce coverage test
     */
    man_buff.size = MAX_BUFFER_SIZE;
    man_buff.data = (char*)calloc(MAX_BUFFER_SIZE, sizeof(char));
    mod_buff.size = MAX_BUFFER_SIZE;
    mod_buff.data = (char*)calloc(MAX_BUFFER_SIZE, sizeof(char));
    if (NULL == pal_system_man_get_fn ||
        (pal_system_man_get_fn(&man_buff) != RESULT_SUCCESS))
    {
        if (man_buff.data) {
            strncpy(man_buff.data, "\"Unknown manufacturer\"", man_buff.size);
        }
    }
    if (NULL == pal_system_mod_get_fn ||
        (pal_system_mod_get_fn(&mod_buff) != RESULT_SUCCESS))
    {
        if (mod_buff.data) {
            strncpy(mod_buff.data, "\"Unknown model\"", mod_buff.size);
        }
    }
    message_descriptor.state = FUMO_DEVICE_STATE_UPDATE_FAILED;
    message_descriptor.message_type = emt_pop_up;
    message_descriptor.message_mode = emm_normal;
    message_descriptor.icon_type = eit_device;
    message_descriptor.header_text = "System update unsuccessful";
    static const char* message_fmt =
        "Sorry, there was a problem updating your %s %s.\n"
        "No changes were made";
    const int message_sz = strlen(message_fmt) +
        STRLEN_OR_NULL(mod_buff.data) +
        STRLEN_OR_NULL(man_buff.data) + 1;
    message_descriptor.message_text =
        (char*) calloc(message_sz, sizeof(char));
    if (message_descriptor.message_text) {
        snprintf(message_descriptor.message_text, message_sz, message_fmt,
            STR_OR_NULL(man_buff.data), STR_OR_NULL(mod_buff.data));
    } else {
        WRN("message_descriptor.message_text is null");
        rc = MO_ERROR_DEVICE_FULL;
    }

    if (!man_buff.data || !mod_buff.data) {
        WRN("message_descriptor.message_text (!man_buff.data || !mod_buff.data)");
        rc = MO_ERROR_DEVICE_FULL;
    }

    message_descriptor.button_type = ebt_ok;
    if (gui_create_message_func) {
        DBG("call PAL function");
        gui_create_message_func(&message_descriptor);
    } else {
        WRN("pal_fumo_gui_create_message was not loaded");
    }
    FREE_IF(man_buff.data);
    FREE_IF(mod_buff.data);
    FREE_IF(message_descriptor.message_text);
    return rc;
}

/**
    @brief it calls update from PAL
    @param [in] fta arguments from thread
    @return 0 if success
*/
int update(fumo_thread_args *fta)
{
    (void)fta;
    int rc = 0;
    unsigned long defer_time = 0;
    int automatic_update_enable = 0;
    int state = 0;

    /** @todo fumo_node* fn_pkgdata_node = NULL; */
    ft_update_firmware update_firmware_func = NULL;
    ft_pal_fumo_gui_destroy_message destroy_message_func = NULL;

    DM_LOGI("FUMO: check SU cancel before update %d:%d",download_cancel_flag,
                                                        update_cancel_flag);
    if( update_cancel_flag || download_cancel_flag) {
        /* SU Cancel came prior to the Update message to the end-user initiating */
        update_cancel_flag = 0;
        download_cancel_flag = 0;
        return FUMO_RC_PRV_SU_CANCEL;
    }
    if(NULL == (update_firmware_func = get_pal_method("pal_update_firmware")))
        return 1;

    /* @todo PkgData node searching
    if(NULL == (fn_pkgdata_node=find_node_by_uri(
                                fta->node_->uri_, FUMO_URI_PKGDATA))) {
        DM_LOGE("FUMO: update, Can't find node by URI: %s",
            fta->node_->uri_);
        return 1;
    } */

    memset(&g_pal_update_descriptor, 0, sizeof(g_pal_update_descriptor));

    if(MO_ERROR_NONE !=
        (rc=fumo_storage_get_update_descriptor(&g_pal_update_descriptor))) {
        DM_LOGE("FUMO: update, Can't get update descriptor, rc=%d", rc);
        dlclose(pal_handle_update);
        pal_handle_update = NULL;
        return 1;
    }

    //truncate checksum here

    g_pal_update_descriptor.progress = update_progress;
    rc = fumo_storage_get_state(&state);
    if(rc) {
        DM_LOGI("FUMO: update: can't get fumo state [%d]",rc);
        return rc;
    }
     DM_LOGI("FUMO: update: fumo state = %d",state);
    if(gui_supported) {
#ifdef ANDROID
        if(state == FUMO_DEVICE_STATE_UPDATE_DEFERRED) {
            if(MO_ERROR_NONE != fumo_storage_get_long_value(
                    offsetof(fumo_storage_t, defer_time), &defer_time)) {
                DM_LOGE("FUMO: Can not get defer_time from fumo_state_storage");
                return rc;
            }
            if(MO_ERROR_NONE != (rc=fumo_storage_get_int_value(
                    offsetof(fumo_storage_t, automatic_update_enable), &automatic_update_enable))) {
                DM_LOGE("FUMO: get automatic_update_enable update rc = %d", rc);
                return rc;
            }
            struct timespec ts_defer;
            clock_gettime(CLOCK_REALTIME, &ts_defer);
            if(ts_defer.tv_sec >= defer_time) {
                DM_LOGE("FUMO: defer timer expired, wait 3 minutes and show post-download dialog or automatically");
                sleep(3*60);
            }
            else
            {
                DM_LOGE("FUMO: need restart timer");
                int rcc = wait_defer_timer(defer_time);
                if(rcc != 0 || ebt_check_for_update == gui_response.button_id) {
                    DM_LOGI("FUMO: Timeout expired, show post-download dialog or automatically");
                } else {
                    DM_LOGI("FUMO: Defer update cancel");
                    return 0;
                }
            }
        }

        do {
            if(automatic_update_enable)
                break;

            if (show_system_update_ready_to_install_notification())
                return 1;
            g_key_deferred = 0;
            // Waiting for User reaction
            wait_for_user_reaction_or_timer(DELAY_24_HOURS);
            if (update_cancel_flag || download_cancel_flag) {
                /* SU Cancel came during the Update message to the end-user initiating */
                DM_LOGI("FUMO: SU cancel came prior to update");
                update_cancel_flag = 0;
                download_cancel_flag = 0;
                return FUMO_RC_PRV_SU_CANCEL;
            }
            if(ebt_cancel == gui_response.button_id) {
                fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                install_by_user_cancel_flag = 1;
                return PAL_RC_FRMW_INSTALL_CANCELLED;
            } else if (ebt_yes == gui_response.button_id &&
                    gui_response.defered_update_time == 0) {
                DM_LOGI("FUMO: User selected Install now");
                if (MO_ERROR_NONE != (rc = check_low_memory_status(UPDATE))) {
                    return rc;
                }
                fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_PROGRESSING);
                break;
            } else if (ebt_yes == gui_response.button_id &&
                    gui_response.defered_update_time > 0) {
                fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_PROGRESSING);
                unsigned long defer_time_by_user = gui_response.defered_update_time / 1000;
                DM_LOGI("FUMO: User select Update later at %ld", defer_time);
                DM_LOGI("FUMO: Current time - %ld", time(NULL));
                long delay = defer_time_by_user - time(NULL);
                if (delay < 0) {
                    DM_LOGI("FUMO: Update will start tomorrow");
                    defer_time_by_user = time(NULL) + DELAY_24_HOURS;
                }
                rc = fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_DEFERRED);
                if(MO_ERROR_NONE != rc) {
                    DM_LOGE("FUMO: can't set fumo state' rc = %d", rc);
                    return rc;
                }
                if(MO_ERROR_NONE != (rc=fumo_storage_set_int_value(
                        offsetof(fumo_storage_t, automatic_update_enable), gui_response.automatic_update_enable))) {
                    DM_LOGE("FUMO: set automatic_update_enable update rc = %d", rc);
                    return rc;
                }
                DM_LOGI("FUMO: Waiting until %ld sec ", defer_time_by_user);
                int rcc = wait_defer_timer(defer_time_by_user);
                if(rcc != 0 || ebt_check_for_update == gui_response.button_id)
                {
                    DM_LOGI("FUMO: Timeout expired, Start update...");
                    fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_PROGRESSING);
                    DM_LOGI("FUMO: automatic update unable: %d", gui_response.automatic_update_enable);
                    if(gui_response.automatic_update_enable) {
                        DM_LOGI("FUMO: gui_response.automatic_update_enable break");
                        break;
                    }
                } else {
                    DM_LOGI("FUMO: Defer update cancel");
                    if (update_cancel_flag || download_cancel_flag)
                        rc = FUMO_RC_PRV_SU_CANCEL_BEFORE_DEFERRED_EVENT;
                    else
                        rc = 0;
                    update_cancel_flag = 0;
                    download_cancel_flag = 0;
                    return rc;
                }
            } else if (ebt_oom_notification == gui_response.button_id){
                DM_LOGI("FUMO: POST DOWNLOAD ebt_oom_notification");
            }
        } while(1);
#endif
    }

    fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_RESTART);
	
    if(MO_ERROR_SUCCESS !=
        (rc=update_firmware_func(&g_pal_update_descriptor))) {
        DM_LOGE("FUMO: update, Can't call PAL update, rc=%d", rc);
        show_system_update_failed();
        dlclose(pal_handle_update);
        pal_handle_update = NULL;
        return PAL_RC_FRMW_DOWNLOAD_ERROR_VERIFICATION_FAILED==rc? rc : PAL_RC_FRMW_DOWNLOAD_ERROR;
    }
    DM_LOGI("FUMO: check SU cancel during undate %d:%d",update_cancel_flag,download_cancel_flag);
    if( update_cancel_flag || download_cancel_flag) {
        // send alarm to client, SU Cancel SMS shall have no effect on the Update session
        send_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_NO_FIRMWARE,
                             alarm_type_downloadandupdate);
    }
    if (gui_supported) {
#ifdef ANDROID
        show_installing_system_update_notification();
        /* Waiting for update complete */
        DM_LOGI("FUMO: Start waiting for update complete");
        pthread_mutex_lock(&update_wait_mutex);
        pthread_cond_wait(&update_wait_conditional, &update_wait_mutex);
        pthread_mutex_unlock(&update_wait_mutex);
        DM_LOGI("FUMO: End waiting for update complete");

        if(NULL == (destroy_message_func =
                get_pal_method("pal_fumo_gui_destroy_message")))
            return 1;

        DM_LOGI("FUMO: Hide Installing system update notification");
        destroy_message_func(FUMO_DEVICE_STATE_UPDATE_PROGRESSING);

        if (g_update_state == PAL_RC_FRMW_UPD_COMPLETED_SUCCESS) {
            DM_LOGI("FUMO: Update completed successfully");
            check_low_battery_status();
            do {
                show_system_update_ready_restart_notification();
                g_key_deferred = 0;
                // Waiting for User reaction
                wait_for_user_reaction_or_timer(DELAY_24_HOURS);

                if (ebt_yes == gui_response.button_id) {
                    show_update_complete_dialog();
                    break;
                }
            } while(1);
        }
#endif
    }
    DM_LOGI("FUMO: check SU cancel during or after update %d:%d",download_cancel_flag,
                                                       update_cancel_flag);
    if( update_cancel_flag || download_cancel_flag)
        rc = FUMO_RC_PRV_SU_CANCEL_AFTER_UPDATE;
    else
        rc = 0;

    DM_LOGI("FUMO: update }");

    update_cancel_flag = 0;
    download_cancel_flag = 0;
    return rc;
}


/** @todo do update cancel  */
int fumo_update_cancel(void)
{
    update_cancel_flag = 1;
    int state = 0;
    fumo_storage_get_state(&state);
    DM_LOGI("FUMO: update cancel state = %d", state);
    pthread_mutex_lock(&defer_wait_mutex);
    pthread_cond_signal(&defer_wait_conditional);
    pthread_mutex_unlock(&defer_wait_mutex);
    return MO_ERROR_NONE;
}

/**
 * @brief Shows dialog "System update available"
 * @param message predownload message
 * @param url predownload url
 * @param over_wifi download via Wi-Fi, 0 - no, 1 - yes
 * @return 0 if success
 */
int show_system_update_available_notification(char* message, char* url, int over_wifi)
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if(NULL == (gui_create_message_func =
            get_pal_method("pal_fumo_gui_create_message")))
        return 1;

    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
                    malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return 1;
    }

    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->message_type = emt_pop_up;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_device;
    message_descriptor->header_text = strdup("System update available");
    if (message != NULL)
        message_descriptor->message_text = strdup(message);
    if(url != NULL) {
        message_descriptor->hyper_link_caption = strdup("Learn More Online");
        message_descriptor->hyper_link = strdup(url);
    }
    if(over_wifi == 0)
    {
        message_descriptor->state = FUMO_DEVICE_STATE_UPDATE_AVALIABLE;
        message_descriptor->button_type = ebt_no & ebt_later & ebt_yes;
    }
    else
    {
        message_descriptor->state = FUMO_DEVICE_STATE_DOWNLOAD_OVER_WIFI;
        message_descriptor->button_type = ebt_no & ebt_yes;
    }

    message_descriptor->user_reaction = on_gui_response;

    DM_LOGI("FUMO: Show dialog <System update available>");
    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);
    return 0;
}

/**
 * @brief Shows dialog "Downloading system update"
 * @param message predownload message
 * @return 0 if success
 */
int show_downloading_system_update_notification(char* message)
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if(NULL == (gui_create_message_func =
            get_pal_method("pal_fumo_gui_create_message")))
        return 1;

    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
                    malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return 1;
    }

    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->state = FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING;
    message_descriptor->message_type = emt_notification;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_device;
    message_descriptor->header_text = strdup("Downloading system update");
    if (message != NULL)
        message_descriptor->message_text = strdup(message);
    message_descriptor->progress_bar_caption = strdup("Downloading");
    message_descriptor->button_type = ebt_ok;

    DM_LOGI("FUMO: Show notification <Downloading system update>");
    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);
    return 0;
}
/**
    @brief Get download descriptor, check available space
    @param [in] fta arguments from thread
    @param [in] url
    @return 0 if success
*/
int pre_download(fumo_thread_args *fta, char* url, char* severity)
{
    int rc = 1;
    unsigned long download_size = 0;
    int alarm = MO_ERROR_COMMAND_FAILED;

    DM_LOGI("FUMO: SU cancel check, before pre_download %d",download_cancel_flag);
    if(download_cancel_flag) {
         download_cancel_flag = 0;
         return FUMO_RC_PRV_SU_CANCEL;
    }

    /** Get download descriptor */
    if(NULL == (request_download_descriptor_func =
            (ft_request_download_descriptor)get_pal_method("pal_request_download_descriptor")))
        return rc;

    if(NULL == (create_download_descriptor_func =
            (ft_create_download_descriptor)get_pal_method("pal_create_download_descriptor")))
        return rc;

    int retry_number = 0;
    int rcc = 1;
    while(retry_number < 2) {
        rc = request_download_descriptor_func(url);
        DM_LOGE("FUMO: Download descriptor downloaded return %d, retry_number=%d",
                 rc, retry_number);
        rcc = create_download_descriptor_func(NULL, &download_descriptor);
        if(rcc != PAL_RC_FRMW_DOWNLOAD_SUCCESS) {
            DM_LOGI("FUMO: create_download_descriptor_func return  %d rc = %d",rcc, rc);
            retry_number++;
            if(retry_number == 1) {
                DM_LOGI("FUMO: download wait 30 seconds for start first retry attempt");
                sleep(30);
                DM_LOGI("FUMO: download start first retry attempt");
             } else {
                 DM_LOGI("FUMO: download wait 3 minutes for start second retry attempt");
                 sleep(3*60);
                 DM_LOGI("FUMO: download start second retry attempt");
            }
        } else
            break;
    }
    if(rcc != PAL_RC_FRMW_DOWNLOAD_SUCCESS) {
        DM_LOGI("FUMO:  create_download_descriptor fail %d",rc);
        return rc;
    } else
        DM_LOGI("FUMO: create_download_descriptor_func done");

    fumo_storage_set_download_descriptor(download_descriptor);

    /** Get required size */
    if (download_descriptor->size != 0 ){
        download_size = download_descriptor->size;
    } else {
        download_size = FUMO_DOWNLOAD_FILE_SIZE;
    }

    if (download_size > FUMO_MAX_PKG_SIZE) {
        DM_LOGE("FUMO: Package size more than 2Gb %lu", download_size);
        alarm = FUMO_RC_UPDATE_PACKAGE_NOT_ACCEPTABLE;
        send_alarm_to_client(fta, alarm, alarm_type_download);
        fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_FAILED);
        return rc;
    }

    if (MO_ERROR_NONE != (rc = check_low_memory_status(DOWNLOAD))) {
        return rc;
    }

    DM_LOGI("FUMO: SU cancel check, after pre_download %d",download_cancel_flag);
    if(download_cancel_flag)
        rc = FUMO_RC_PRV_SU_CANCEL;
    else
        rc = MO_ERROR_NONE;

    return rc;
}


/**
    @brief to show dialog if wifi_only = false
    @param [in] fta arguments from thread
    @param [in] severity update severity
    @param [in] url url for download
    @return RESULT_SUCCESS if success
            error code otherwise
*/
int downloading_wifi_only_false(fumo_thread_args *fta, char *severity, char *url)
{
    int rc = RESULT_ERROR;
    int wifi_only_settings = 0;
    download_descriptor = NULL;
    DM_LOGI("FUMO: download check SU cancel before download %d",
                                                        download_cancel_flag);
    if(download_cancel_flag) {
        return FUMO_RC_PRV_SU_CANCEL;
    }

    if (NULL == (pal_fumo_policy_check = (ft_policy_check) get_pal_method(
                    "pal_policy_check")))
       return rc;

    /** Get methods from pal */
    if (NULL == (wifi_status_get = (ft_network_wifi_status_get) get_pal_method(
            "pal_network_wifi_status_get")))
        return rc;

    if (NULL == (wifi_state_get = (ft_wifi_state_get) get_pal_method(
            "pal_network_wifi_state_get")))
        return rc;

    if(NULL == (download_firmware_func =
            (ft_download_firmware)get_pal_method("pal_download_firmware")))
        return rc;

    /** wifionly verification */
    DM_LOGE("FUMO: WIFIONLY verification...");
    if (MO_ERROR_NONE != fumo_storage_get_int_value(
            offsetof(fumo_storage_t, wifi_only),
            &wifi_only_settings) && 0 != wifi_only_settings) {
        DM_LOGE("FUMO: fumo_storage_get_int_value error");
    }

    do {
        if(0 !=(rc = pre_download(fta, url, severity)))
            return rc;

        // Show "System update available" dialog
        if(show_system_update_available_notification(
                download_descriptor->preDownloadMessage,
                download_descriptor->preDownloadURL,
                wifi_only_settings))
            return rc;

        // Waiting for User reaction
        g_key_deferred = 0;
        wait_for_user_reaction_or_timer(DELAY_24_HOURS);

        DM_LOGI("FUMO: check SU cancel %d", download_cancel_flag);
        if(download_cancel_flag) {
            download_cancel_flag = 0;
            return FUMO_RC_PRV_SU_CANCEL;
        }

        if(ebt_no == gui_response.button_id) {
            fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
            download_by_user_cancel_flag = 1;
            break;
        } else if(ebt_yes == gui_response.button_id &&
            gui_response.defered_update_time == 0) {
            DM_LOGI("FUMO: User pressed Yes button");
            fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
            show_downloading_system_update_notification(
                download_descriptor->preDownloadMessage);
            return RESULT_SUCCESS;
        } else if(ebt_later == gui_response.button_id) {
            DM_LOGI("FUMO: user pressed Install later ");
            g_key_deferred = 0;
            fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_DEFERRED);
            // waiting for user select deferred time
            wait_for_user_reaction_or_timer(DELAY_24_HOURS);
            if(ebt_yes == gui_response.button_id &&
                    gui_response.defered_update_time > 0) {
                DM_LOGI("FUMO: gui_response.wifi_requred  = %d", gui_response.wifi_requred);
                if(gui_response.wifi_requred) {
                     if(get_wifi_state() == 0)
                     {
                         fumo_storage_set_state(FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED);
                         int rcc = wait_wifi();
                         if(rcc != 0)
                         {
                             DM_LOGI("FUMO: Wi-Fi timer expires, show pre-download dialog");
                         }
                         else
                         {
                             DM_LOGI("FUMO: Wi-Fi connected, start download");
                             DM_LOGI("FUMO: wifi_is_set = %d", get_wifi_state());
                             fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
                             return RESULT_SUCCESS;
                         }
                     }
                 } else {
                     unsigned long defer_time_by_user = gui_response.defered_update_time / 1000;
                     DM_LOGI("FUMO: User select Download later at %ld", defer_time_by_user);
                     DM_LOGI("FUMO: Current time - %d", time(NULL));
                     fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_DEFERRED);
                     long delay = defer_time_by_user - time(NULL);
                     if (delay < 0) {
                         DM_LOGI("FUMO: Download will start tomorrow");
                         defer_time_by_user = time(NULL) + DELAY_24_HOURS;
                     }

                     DM_LOGI("FUMO: Waiting until %d sec ", defer_time_by_user);

                     int rcc = wait_defer_timer(defer_time_by_user);
                     if(rcc != 0  || ebt_check_for_update == gui_response.button_id) {
                         DM_LOGI("FUMO: defer_timer rcc = %d button_id = %d",rcc,gui_response.button_id);
                         fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
                         DM_LOGI("FUMO: Timeout expired, show pre-download dialog");
                     } else {
                         DM_LOGI("FUMO: Defer download cancel");
                         if (download_cancel_flag)
                             rc = FUMO_RC_PRV_SU_CANCEL_BEFORE_DEFERRED_EVENT;
                         else
                             rc = RESULT_SUCCESS;
                         download_cancel_flag = 0;
                         return rc;
                     }
                 }
            } else {
                DM_LOGI("FUMO: User pressed key %d",gui_response.button_id);
            }
        }
    } while(1);
    return RESULT_ERROR;
}

static void * pal_handle = NULL;
typedef int (*pal_request_admin_network_t)(bool);
static pal_request_admin_network_t pal_request_admin_network_fn = NULL;

static int admin_apn_status = ADMIN_NET_UNKNOWN;

void netm_up_admin_network()
{
    int try_time = 0;
    if (ADMIN_NET_AVAILABLE == admin_apn_status) {
        DM_LOGD("NM: admin apn already established");
        return;
    }
        DM_LOGD("NM: Try to switch ON admin apn");
        pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
        if(pal_handle == NULL) {
            DM_LOGE("NM: can't open PAL lib %s", dlerror());
            // dmclient shall continue work
        } else {
            pal_request_admin_network_fn = dlsym(pal_handle,
                    PAL_REQUEST_ADMIN_APN);
            if(pal_request_admin_network_fn == NULL) {
                DM_LOGE("NM: pal_request_admin_network not found!");
                // dmclient shall continue work
            } else {
                admin_apn_status = pal_request_admin_network_fn(true);
                DM_LOGD("NM: SSSwitch ON admin apn - status is [%d]", admin_apn_status);
            }

        }
}

void netm_down_admin_network()
{
    if (ADMIN_NET_AVAILABLE == admin_apn_status) {
        admin_apn_status = pal_request_admin_network_fn(false);
        DM_LOGE("NM: Switch OFF admin apn - status is [%d]", admin_apn_status);
    }
}


int request_admin_network()
{
	void * pal_handle = NULL;
	pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
	if(pal_handle == NULL) {
		DM_LOGE("NM: can't open PAL lib");
		return -1;
	}
	int (*testFunc)() = dlsym(pal_handle, "get_network_status");
	int net_status=0;

	if(testFunc == NULL) {
		DM_LOGE("NM: testFunc not found!");
		return -1;
	}
	net_status = testFunc();
	DM_LOGD("NM: net_status status is [%d]", net_status);
	if(net_status == 2 || net_status == 3)
	{
		netm_down_admin_network();
		DM_LOGD("NM: ydc test off -> ON admin apn - status");
		netm_up_admin_network();
	} else if (net_status == 0)
	{
		netm_up_admin_network();
	}

	net_status = testFunc();
	DM_LOGD("NM: after net_status status is [%d]", net_status);
	return 0;
	//return net_status;
}


/**
    @brief it calls download from PAL
    @param [in] fta arguments from thread
    @return 0 if success
*/
int download(fumo_thread_args *fta)
{
    int rc = RESULT_ERROR;
    pal_download_status_t download_status;
    ft_get_current_network get_current_network_func = NULL;
    fumo_node* fn_pkgurl_node = NULL;
    char *severity = NULL;
    char *url = NULL;
    unsigned int url_size = 0;
    int current_network = 0;
    int wifi_only_settings = 0;
    int restart_download = 0;
    unsigned long defer_time = 0;
    int state = 0;
    download_descriptor = NULL;

    DM_LOGI("FUMO: download check SU cancel before download %d",
                                                        download_cancel_flag);
    if(download_cancel_flag) {
        /* SU cancel came prior to download message the end-user initiating */
        download_cancel_flag = 0;
        return FUMO_RC_PRV_SU_CANCEL;
    }

    if (NULL == (pal_fumo_policy_check = (ft_policy_check) get_pal_method(
                    "pal_policy_check")))
       return rc;

    /** Get methods from pal */
    if (NULL == (wifi_status_get = (ft_network_wifi_status_get) get_pal_method(
            "pal_network_wifi_status_get")))
        return rc;

    if (NULL == (wifi_state_get = (ft_wifi_state_get) get_pal_method(
            "pal_network_wifi_state_get")))
        return rc;

    if(NULL == (download_firmware_func =
            (ft_download_firmware)get_pal_method("pal_download_firmware")))
        return rc;

    if(NULL == (get_current_network_func =
            (ft_get_current_network)get_pal_method("pal_network_currentnetwork_get")))
        return rc;

    /** Get node value */
    if(NULL == (fn_pkgurl_node=find_node_by_uri(fta->node_->uri_,
        FUMO_URI_PKGURL))) {
        DM_LOGE("FUMO: Can't get node by uri %s/%s",
        fta->node_->uri_, FUMO_URI_PKGURL);
        return rc;
    }

    if(MO_ERROR_NONE != get_node_value(fn_pkgurl_node,
            &url_size, &url) || url_size == 0) {
        DM_LOGE("FUMO: uri %s/%s hasn't value",
        fta->node_->uri_, FUMO_URI_PKGURL);
        return rc;
    }
    char * pkg_name = NULL;
    fumo_storage_get_pkg_name(&pkg_name);
    int res = pal_fumo_policy_check(pkg_name,PALFU_WORK_PATH);
    if(MO_ERROR_NONE == res || MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED == res ){
        DM_LOGE("FUMO: Enterprise policy check %d", res);

    } else {
        DM_LOGE("FUMO: Enterprise policy check failed %d", rc);
        res = FUMO_ENTERPRISE_POLICY_REJECT;
        send_alarm_to_client(fta, rc, alarm_type_download);
        return rc;
    }

    /** wifionly verification */
    DM_LOGE("FUMO: WIFIONLY verification...");
    if (MO_ERROR_NONE != fumo_storage_get_int_value(
            offsetof(fumo_storage_t, wifi_only),
            &wifi_only_settings) && 0 != wifi_only_settings) {
        DM_LOGE("FUMO: fumo_storage_get_int_value error");
    }

    DM_LOGI("FUMO: wifi_only_settings = %d", wifi_only_settings);
    DM_LOGI("FUMO: wifi_is_set = %d", get_wifi_state());

    /** Get Severity */
    if(MO_ERROR_NONE != fumo_storage_get_string_value(
            offsetof(fumo_storage_t, severity),
            NAME_MAX, &severity)) {
        DM_LOGE("FUMO: Can not get severity from fumo_state_storage");
        return rc;
    }
    DM_LOGI("FUMO: Severity = %s", severity);

    /** Get restard_download flag */
    if(MO_ERROR_NONE != fumo_storage_get_int_value(
            offsetof(fumo_storage_t, restart_download), &restart_download)) {
        DM_LOGE("FUMO: Can not get restard_download from fumo_state_storage");
        return rc;
    }
    DM_LOGI("FUMO: restart_download = %d", restart_download);
    DM_LOGI("FUMO: gui_supported = %d", gui_supported);

    rc = fumo_storage_get_state(&state);
    if(MO_ERROR_NONE != rc) {
        DM_LOGE("FUMO: Can not get state from fumo_state_storage");
        return rc;
    }
    DM_LOGI("FUMO: download: fumo state = %d",state);

    if(state == FUMO_DEVICE_STATE_UPDATE_DEFERRED || state == FUMO_DEVICE_STATE_READY_TO_UPDATE) {  //modified by chapin
        DM_LOGE("FUMO: Download success, waiting defer timer for update");
        return MO_ERROR_NONE;
    }
    if(0 != (rc = pre_download(fta, url, severity))) {
        //return MO_ERROR_COMMAND_FAILED;
		return rc; //modified by chapin
    }
    if(gui_supported && !restart_download) {
#ifdef ANDROID
        if(state == FUMO_DEVICE_STATE_DOWNLOAD_DEFERRED) {
            if(MO_ERROR_NONE != fumo_storage_get_long_value(
                    offsetof(fumo_storage_t, defer_time), &defer_time)) {
                DM_LOGE("FUMO: Can not get defer_time from fumo_state_storage");
                return rc;
            }
            struct timespec ts_defer;
            clock_gettime(CLOCK_REALTIME, &ts_defer);
            if(ts_defer.tv_sec >= defer_time) {
                DM_LOGE("FUMO: defer timer expired, wait 3 minutes and show pre-download dialog");
                sleep(3*60);
            }
            else
            {
                DM_LOGE("FUMO: need restart timer");
                int rcc = wait_defer_timer(defer_time);
                if(rcc != 0 || ebt_check_for_update == gui_response.button_id) {
                    DM_LOGI("FUMO: Timeout expired, show pre-download dialog");
                } else {
                    DM_LOGI("FUMO: Defer download cancel");
                    return 0;
                }
            }
        }

        if(severity_optional == atoi(severity))
        {
            if(wifi_only_settings == 0)
            {
                if (0 != (rc = downloading_wifi_only_false(fta,severity,url))) {
                    return rc;
                }
            }
            else
            {
                // Show "System update available" dialog
                if(show_system_update_available_notification(
                    download_descriptor->preDownloadMessage,
                    download_descriptor->preDownloadURL,
                    wifi_only_settings))
                return MO_ERROR_COMMAND_FAILED;

                // Waiting for User reaction
                wait_for_user_reaction();

                if(ebt_cancel == gui_response.button_id)
                {
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    download_by_user_cancel_flag = 1;
                    return MO_ERROR_OPERATION_CANCELED;
                }
                if(ebt_yes == gui_response.button_id)
                {
                    if(get_wifi_state() == 0) {
                        fumo_storage_set_state(FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED);
                        show_gui_system_update_wifi_not_connected();
                        int rcc = wait_wifi();
                        hide_gui_system_update_wifi_not_connected();

                       if(rcc == 0)
                       {
                            DM_LOGI("FUMO: Wi-Fi connected, start download");
                            DM_LOGI("FUMO: wifi_is_set = %d", get_wifi_state());
                       }
                       else
                       {
                            DM_LOGI("FUMO: Wi-Fi timer expires, download isn't completed");
                            wifi_only_timer_expired_flag = 1;
                            return MO_ERROR_OPERATION_CANCELED;
                       }
                    }
                }
            }
        }
        else if(severity_mandatory == atoi(severity))
        {
            if (wifi_only_settings == 1) {
                if (get_wifi_state() == 0) {
                    fumo_storage_set_state(FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED);
                    if (MO_ERROR_NONE != (rc = check_low_memory_status(DOWNLOAD))) {
                        return rc;
                    }
                    if (get_wifi_state() == 0) {
                        show_gui_system_update_wifi_not_connected();
                        int rcc = wait_wifi();
                        hide_gui_system_update_wifi_not_connected();

                        if (rcc == 0) {
                            DM_LOGI("FUMO: Wi-Fi connected, start download");
                            fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
                            DM_LOGI("FUMO: wifi_is_set = %d", get_wifi_state());
                        } else if(rcc == 1) {
                            DM_LOGI("FUMO: Check for update, WiFi only false");
                        } else {
                            DM_LOGI("FUMO: Wi-Fi timer expires, download isn't completed");
                            wifi_only_timer_expired_flag = 1;
                            fumo_storage_reset_wifi();
                            return rc;
                        }
                    }
                }
            }
        }
#endif
    }
    else if(!gui_supported) {
        fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
        if(wifi_only_settings == 1) {
            if(get_wifi_state() == 0)
            {
                fumo_storage_set_state(FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED);
                int rcc = wait_wifi();
                if(rcc != 0)
                {
                    DM_LOGI("FUMO: Wi-Fi timer expires, download isn't completed");
                    wifi_only_timer_expired_flag = 1;
                    fumo_storage_reset_wifi();
                    return rc;
                }
                else
                {
                    DM_LOGI("FUMO: Wi-Fi connected, start download");
                    fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
                    DM_LOGI("FUMO: wifi_is_set = %d", get_wifi_state());
                }
            }
        }
    }

    DM_LOGI("FUMO: check SU cancel in download %d", download_cancel_flag);
    if(download_cancel_flag) {
        download_cancel_flag = 0;
        return FUMO_RC_PRV_SU_CANCEL;
    }

    // Downloading...
    memset(&download_status, 0, sizeof(download_status));
    download_status.serverUrl = url;
    download_status.wifi_only = wifi_only_settings;
    DM_LOGI("FUMO: Start downloading from %s", download_status.serverUrl);
    download_status.on_progress = download_on_progess;

    fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
	create_network_up_flag();
    if(gui_supported) {
    	DM_LOGI("FUMO:  show_downloading_system_update_notification\n");
        show_downloading_system_update_notification(download_descriptor->preDownloadMessage);
    	DM_LOGI("FUMO:  show_downloading_system_update_notification finish\n");
    }
    /** only LTE coverage area */
    if (0 == get_wifi_state()) {
        if (0 != (rc = download_failed_retry(&download_status, fta, severity))) {
            DM_LOGE("FUMO: download failed with error=%d", rc);
            return rc;
        }
    } else {
        if (0 != (rc = download_failed_retry(&download_status, fta, severity))) {
            DM_LOGE("FUMO: download failed with error=%d", rc);

            if(NETWORK_TYPE_UNKNOWN !=
                get_current_network_func(&current_network)) {
                DM_LOGE("FUMO: get current network func failed with error");
            }

            if(current_network == NETWORK_TYPE_LTE) {
                continue_download_via_lte = 1;
                fumo_storage_set_int_value(offsetof(fumo_storage_t, restart_download), continue_download_via_lte);
            }

            return rc;
        }
    }

    fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_COMPLETE);
    fumo_storage_set_int_value(offsetof(fumo_storage_t, restart_download), 0);
    DM_LOGI("FUMO: Download complete");

    DM_LOGI("FUMO: check SU cancel after download %d", download_cancel_flag);
    if(download_cancel_flag) {
        /* SU cancel came prior to Update message to the end-user initiating */
        download_cancel_flag = 0;
        return FUMO_RC_PRV_SU_CANCEL;
    }

    return rc;
}

int download_failed_retry(pal_download_status_t *download_status, fumo_thread_args *fta, char* severity){
int rc = MO_ERROR_NONE;
int rcc = MO_ERROR_NONE;
    int retry_number = 0;
    int ret;
    int32_t net_type;
    struct timespec ts_retry;

    /*if(MO_ERROR_NONE != fumo_storage_get_int_value(
            offsetof(fumo_storage_t, retry_count), &retry_number)) {
        DM_LOGE("FUMO: Can not get retry_count from fumo_state_storage");
        return MO_ERROR_COMMAND_FAILED;
    }*/
    /** Get methods from pal */
    if (NULL == (network_type_get = (ft_network_type) get_pal_method(
            "pal_network_connecttype_get"))) {
        return MO_ERROR_COMMAND_FAILED;
    }

    DM_LOGI("FUMO: retry_number = %d", retry_number);

    while(retry_number < 800) {  //wait about 13 hour
        if (RESULT_SUCCESS != network_type_get(&net_type)) {
			DM_LOGI("network_type_get = %d, net_type = %d\n", network_type_get(&net_type), net_type);
            return MO_ERROR_COMMAND_FAILED;
        }

        if ((net_type == CONNECT_TYPE_WIFI) ||
            (net_type == CONNECT_TYPE_MOBILE && download_status->wifi_only != 1)) {
            DM_LOGI("FUMO: net_type = %d", net_type);
            DM_LOGI("FUMO: in while retry_number = %d", retry_number);
            if(0 != (rc=download_firmware_func(download_status))) {
                if (rc== PAL_RC_FRMW_DOWNLOAD_ERROR_VERIFICATION_FAILED)
                {
                    DM_LOGE("FUMO: download failed with error=%d", rc);
                    break;
                } else if (rc == PAL_RC_FRMW_DOWNLOAD_ERROR_MEMORY)
				{
					DM_LOGE("FUMO: download low memory");
					break;
				}
                DM_LOGE("FUMO: download failed retry_number=%d with error=%d", retry_number, rc);
				
                /*if (MO_ERROR_NONE != (rcc = check_low_memory_status(DOWNLOAD))) {
                    return rcc;
                }*/
                if(retry_number == 0) {
                    DM_LOGI("FUMO: download wait 10 seconds for start first retry attempt");

                    clock_gettime(CLOCK_REALTIME, &ts_retry);
                    ts_retry.tv_sec += 10;
                    ts_retry.tv_nsec = 0;
                    pthread_mutex_lock(&retry_timeout_mutex);
                    ret = pthread_cond_timedwait(&retry_timeout, &retry_timeout_mutex, &ts_retry);
                    pthread_mutex_unlock(&retry_timeout_mutex);
                    if(ret == ETIMEDOUT){
                        DM_LOGI("FUMO: download start first retry attempt");

                    } else {
                        if (download_status->wifi_only == 1 ) {
                            if( 0 != (rcc = wifi_connect_timedwait())) {
                                fumo_storage_set_int_value(offsetof(fumo_storage_t, retry_count), 0);
                                rc = PAL_RC_FRMW_DOWNLOAD_WIFI_TIMEOUT;
                                break;
                            }
                        } else {
                            fumo_storage_set_int_value(offsetof(fumo_storage_t, retry_count), 0);
                            rc = PAL_RC_FRMW_DOWNLOAD_NO_NETWORK;
                            break;
                        }
                    }

                } else if(retry_number > 0){
                    DM_LOGI("FUMO: download wait 1 minutes for start second retry attempt");
                    clock_gettime(CLOCK_REALTIME, &ts_retry);
                    ts_retry.tv_sec += 60;
                    ts_retry.tv_nsec = 0;
                    pthread_mutex_lock(&retry_timeout_mutex);
                    ret = pthread_cond_timedwait(&retry_timeout, &retry_timeout_mutex, &ts_retry);
                    pthread_mutex_unlock(&retry_timeout_mutex);
                    if(ret == ETIMEDOUT){
                        DM_LOGI("FUMO: download start second retry attempt");
                    } else {
                        if (download_status->wifi_only == 1 ) {
                            if( 0 != (rcc = wifi_connect_timedwait())) {
                                fumo_storage_set_int_value(offsetof(fumo_storage_t, retry_count), 0);
                                rc = PAL_RC_FRMW_DOWNLOAD_WIFI_TIMEOUT;
                                break;
                            }
                        } else {
                            fumo_storage_set_int_value(offsetof(fumo_storage_t, retry_count), 0);
                            rc = PAL_RC_FRMW_DOWNLOAD_NO_NETWORK;
                            break;
                        }
                    }
                }
                if (0 == get_wifi_state())
                   request_admin_network();
                retry_number++;
                fumo_storage_set_int_value(offsetof(fumo_storage_t, retry_count), retry_number);

            } else {
                DM_LOGE("FUMO: download retry_number=%d with error=%d", retry_number, rc);
                fumo_storage_set_int_value(offsetof(fumo_storage_t, retry_count), 0);
                break;
            }
        } else if ((net_type == CONNECT_TYPE_MOBILE || net_type == CONNECT_TYPE_UNKNOWN) &&
                   (download_status->wifi_only == 1)) {
            if (MO_ERROR_NONE != (rc = wait_wifi())) {
                fumo_storage_set_int_value(offsetof(fumo_storage_t, retry_count), 0);
                rc = PAL_RC_FRMW_DOWNLOAD_NO_NETWORK;
                break;
            }
        } else {
            fumo_storage_set_int_value(offsetof(fumo_storage_t, retry_count), 0);
            rc = PAL_RC_FRMW_DOWNLOAD_NO_NETWORK;
            break;
        }
        DM_LOGE("FUMO: while exit download retry_number=%d with error=%d", retry_number, rc);
    }
    DM_LOGE("FUMO: download_failed_retry rc = %d", rc);
    return rc;
}

/**
    @brief Wait wifi connect after download failed
    @return 0 if wifi connected, else ETIMEDOUT
*/
int wifi_connect_timedwait()
{
    int rc = 0;
    DM_LOGI("FUMO: PAL_RC_FRMW_DOWNLOAD_NO_NETWORK: WIFI");
    fumo_storage_set_state(FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED);
    rc = wait_wifi();
    if(rc != 0)
    {
        DM_LOGI("FUMO: Wi-Fi timer expires");
        fumo_storage_reset_wifi();
    }
    else
    {
        DM_LOGI("FUMO: Wi-Fi connected, continue download");
        fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
    }
    return rc;
}

/** @todo do download cancel */
int fumo_download_cancel(void)
{
    download_cancel_flag = 1;
    int state = 0;
    fumo_storage_get_state(&state);
    DM_LOGI("FUMO fumo_download_cancel state = %d", state);
    pthread_mutex_lock(&defer_wait_mutex);
    pthread_cond_signal(&defer_wait_conditional);
    pthread_mutex_unlock(&defer_wait_mutex);
    return MO_ERROR_NONE;
}

/**
 *  @brief Type of callback for progress information
 *  @param[in] context of firmware update operation
 *  @param[in] progress progress in update operation
 *  @param[in] err_code error code of update operations
        PAL_RC_FRMW_UPD_....
 *  @return HTTP response status code
 *      101 in progress
 *      200 sucess
 *      500 failed
*/
int download_on_progess(void *context, unsigned int progress, int err_code)
{
    DM_LOGI("FUMO: Download progress: %d", progress);
    DM_LOGI("FUMO: Download err_code: %d", err_code);

    ft_pal_fumo_gui_update_download_progress update_progress_func = NULL;
    ft_pal_fumo_gui_destroy_message destroy_message_func = NULL;
    /** ft_pal_fumo_gui_create_message create_message_func = NULL; */

    if (MO_ERROR_SUCCESS == err_code) {
        if (progress < 100) {
            if(gui_supported && NULL != (update_progress_func =
                    (ft_pal_fumo_gui_update_download_progress)get_pal_method(
                    "pal_fumo_gui_update_download_progress"))) {
                update_progress_func(progress);
            }
            return MO_ERROR_IN_PROGRESS;
        } else { /* completed */
            if(gui_supported && NULL != (destroy_message_func =
                    (ft_pal_fumo_gui_destroy_message)get_pal_method(
                    "pal_fumo_gui_destroy_message"))) {
                destroy_message_func(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
            }
            return MO_ERROR_SUCCESS;
        }
    } else {
        if(gui_supported && NULL != (destroy_message_func =
                (ft_pal_fumo_gui_destroy_message)get_pal_method(
                "pal_fumo_gui_destroy_message"))) {
            destroy_message_func(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
        }
        return MO_ERROR_COMMAND_FAILED;
    }
    return MO_ERROR_SUCCESS;
}

/**
 *  @brief Type of callback for getting User response
 *  @param[in] data pointer to pal_fumo_gui_user_reply_param structure
*/
void on_gui_response(void * data)
{
    memcpy(&gui_response, data, sizeof(pal_fumo_gui_user_reply_param));
    if(gui_response.button_id == ebt_back || gui_response.button_id == ebt_home){
        DM_LOGI("FUMO: User reaction done!  %X",gui_response.button_id);
        g_key_deferred = 1;
    }
    else {
        g_key_deferred = 0;
        pthread_mutex_lock(&gui_wait_mutex);
        pthread_cond_signal(&gui_wait_conditional);
        pthread_mutex_unlock(&gui_wait_mutex);
        DM_LOGI("FUMO: User reaction done!  %X",gui_response.button_id);
    }
}

/**
 *  @brief Helper method. Free memory allocated for message descriptor
 *  @param[in] md pointer to message descriptor
*/
void clean_gui_message_descriptor(pal_fumo_gui_message_descriptor * md)
{
    free(md->header_text);
    md->header_text = NULL;

    free(md->message_text);
    md->message_text = NULL;

    free(md->hyper_link_caption);
    md->hyper_link_caption = NULL;

    free(md->hyper_link);
    md->hyper_link = NULL;

    free(md->progress_bar_caption);
    md->progress_bar_caption = NULL;

    free(md->install_param);
    md->install_param = NULL;

    free(md->severity);
    md->severity = NULL;
}

/**
 *  @brief Show dialog "Pre-Download Memory Check"
 *  @return MO_ERROR_NONE if success
*/
int show_gui_dialog_check_memory()
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if(NULL == (gui_create_message_func =
            (ft_pal_fumo_gui_create_message)get_pal_method("pal_fumo_gui_create_message")))
        return MO_ERROR_COMMAND_FAILED;
    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
        calloc(1, sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return MO_ERROR_COMMAND_FAILED;
    }

    message_descriptor->state = FUMO_DEVICE_STATE_MEMORY_NOT_ENOUGH;
    message_descriptor->message_type = emt_notification;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_device;

    if (MO_ERROR_NONE != fumo_storage_get_long_value(offsetof(fumo_storage_t,
            required_space_for_update), &message_descriptor->required_space_for_update) ||
        MO_ERROR_NONE != fumo_storage_get_long_value(offsetof(fumo_storage_t,
            required_space_for_delete), &message_descriptor->required_space_for_delete)) {
        clean_gui_message_descriptor(message_descriptor);
        free(message_descriptor);
        return MO_ERROR_COMMAND_FAILED;
    }
    message_descriptor->button_type = ebt_no & ebt_choose_files_to_delete;
    message_descriptor->user_reaction = on_gui_response;

    DM_LOGI("FUMO: Show dialog <Pre-Download Memory Check Notification >");

    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);

    return MO_ERROR_NONE;
}

/**
*  @brief Show dialog "Download Unsuccessful"
*  @return MO_ERROR_SUCCESS if success
*/
int show_gui_notification_download_fail()
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if(NULL == (gui_create_message_func =
            (ft_pal_fumo_gui_create_message)get_pal_method("pal_fumo_gui_create_message")))
        return MO_ERROR_COMMAND_FAILED;
    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
        malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return MO_ERROR_COMMAND_FAILED;
    }

    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->state = FUMO_DEVICE_STATE_DOWNLOAD_FAILED;
    message_descriptor->message_type = emt_notification;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_none;
    message_descriptor->header_text = strdup("Download Unsuccessful");
    message_descriptor->message_text = strdup("");

    message_descriptor->hyper_link_caption = strdup("");
    message_descriptor->hyper_link = strdup("");

    message_descriptor->progress_bar_caption = strdup("");
    message_descriptor->button_type = ebt_ok;
    message_descriptor->user_reaction = NULL;

    DM_LOGI("FUMO: Show dialog <Software Download Unsuccessful >");

    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);

    return MO_ERROR_NONE;
}

/**
*  @brief Show dialog "Download Unsuccessful"
*  @return MO_ERROR_SUCCESS if success
*/
int show_gui_notification_fail_check()
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if(NULL == (gui_create_message_func =
            (ft_pal_fumo_gui_create_message)get_pal_method("pal_fumo_gui_create_message")))
        return MO_ERROR_COMMAND_FAILED;
    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
        malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return MO_ERROR_COMMAND_FAILED;
    }

    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->state = FUMO_DEVICE_STATE_DOWNLOAD_FAILED;
    message_descriptor->message_type = emt_notification;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_none;
    message_descriptor->header_text = strdup("Fail integirty check");
    message_descriptor->message_text = strdup("");

    message_descriptor->hyper_link_caption = strdup("");
    message_descriptor->hyper_link = strdup("");

    message_descriptor->progress_bar_caption = strdup("");
    message_descriptor->button_type = ebt_ok;
    message_descriptor->user_reaction = NULL;

    DM_LOGI("FUMO: Show dialog <Software fail integrity check >");

    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);

    return MO_ERROR_NONE;
}

/**
*  @brief Show dialog "System update via Wi-Fi, is not connected"
*  @return MO_ERROR_SUCCESS if success
*/
int show_gui_system_update_wifi_not_connected()
{
    ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
    pal_fumo_gui_message_descriptor * message_descriptor = NULL;

    if(NULL == (gui_create_message_func =
            (ft_pal_fumo_gui_create_message)get_pal_method("pal_fumo_gui_create_message")))
        return MO_ERROR_COMMAND_FAILED;
    if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
        malloc(sizeof(pal_fumo_gui_message_descriptor)))) {
        DM_LOGE("FUMO: node_command_exec: Out of memory");
        return MO_ERROR_COMMAND_FAILED;
    }

    memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
    message_descriptor->state = FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED;
    message_descriptor->message_type = emt_pop_up;
    message_descriptor->message_mode = emm_persistent;
    message_descriptor->icon_type = eit_none;
    message_descriptor->header_text = strdup("System update via Wi-Fi");
    message_descriptor->message_text = strdup("");

    message_descriptor->hyper_link_caption = strdup("");
    message_descriptor->hyper_link = strdup("");

    message_descriptor->progress_bar_caption = strdup("");
    message_descriptor->button_type = ebt_ok;
    message_descriptor->user_reaction = NULL;

    DM_LOGI("FUMO: Show dialog <System update via Wi-Fi >");

    gui_create_message_func(message_descriptor);
    clean_gui_message_descriptor(message_descriptor);

    return MO_ERROR_NONE;
}

/**
 *  @brief Hide dialog "System update via Wi-Fi"
 *  @return MO_ERROR_SUCCESS if success
 */
int hide_gui_system_update_wifi_not_connected()
{
    DBG("()");
    ft_pal_fumo_gui_destroy_message hide_message_fn = NULL;
    if (NULL == (hide_message_fn =
        (ft_pal_fumo_gui_destroy_message)
            get_pal_method("pal_fumo_gui_destroy_message")))
    {
        ERR("load function " "pal_fumo_gui_destroy_message");
        return MO_ERROR_NOT_FOUND;
    }
    int rc = RESULT_SUCCESS;
    if ((rc = hide_message_fn(FUMO_DEVICE_STATE_WIFI_NOT_CONNECTED))
        != RESULT_SUCCESS) {
        ERR("hide_message(via_wifi_only) = %d", rc);
        return MO_ERROR_COMMAND_FAILED;
    }
    DBG("():OK");
    return MO_ERROR_NONE;
}
/**
    @brief entry point for downolad/update thread
    @param [in] args, arguments, fumo_threadArgs actually
    @todo FSM
    @return 0 if success
*/
void* fumo_work_thread(void* args)
{
    fta = (fumo_thread_args*)args;
    long rc = 0;
    int alarm = MO_ERROR_COMMAND_FAILED;
    char * pkg_name = NULL;
	char * severity = NULL;

    free_download_descriptor_func =
            (ft_free_download_descriptor)get_pal_method("free_download_descriptor_t");
    ft_pal_fumo_gui_destroy_message destroy_message_func = NULL;
    destroy_message_func = get_pal_method("pal_fumo_gui_destroy_message");
    if(destroy_message_func == NULL)
        DM_LOGI("FUMO: pal_fumo_gui_destroy_message not found");

	/** Get Severity */
    if(MO_ERROR_NONE != fumo_storage_get_string_value(
            offsetof(fumo_storage_t, severity),
            NAME_MAX, &severity)) {
        DM_LOGE("FUMO: Can not get severity from fumo_state_storage");
    }
    DM_LOGI("FUMO: fumo_work_thread Severity = %s", severity);

    switch(fta->event_) {
        case fte_Download:
        {
			DM_LOGI("fumo_work_thread download start\n");
            int rc = download(fta);
            if(rc) {
                DM_LOGI("FUMO: download state: %d" , rc);
                fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_FAILED);
                alarm = MO_ERROR_COMMAND_FAILED;
                DM_LOGI("FUMO: download failed, event Download");
                if(rc == PAL_RC_FRMW_DOWNLOAD_ERROR_VERIFICATION_FAILED) {
                    if(gui_supported) {
                        show_gui_notification_download_fail();
                    }
                    //alarm = FUMO_RC_NOT_FOUND_OR_UPDATE_FAILED;
                    alarm = FUMO_RC_UPDATE_FAILED;
                    remove_fw_package();
                } else if(rc == PAL_RC_FRMW_DOWNLOAD_SERVER_UNAVAILABLE) {
                    alarm = FUMO_RC_DL_SERVER_IS_UNAVAILABLE;
                    if(gui_supported)
                        show_gui_notification_download_fail();
                } else if(rc == FUMO_RC_PRV_SU_CANCEL) {
                    alarm = FUMO_RC_SU_CANCEL_SUCCESS_UPDATE;
                } else if(rc == PAL_RC_FRMW_DOWNLOAD_PACKAGE_NOT_ACCEPTABLE) {
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    alarm = FUMO_RC_UPDATE_PACKAGE_NOT_ACCEPTABLE;
                } else if (rc == FUMO_RC_PRV_SU_CANCEL_BEFORE_DEFERRED_EVENT) {
                    alarm = FUMO_RC_SU_CANCEL_SUCCESS_DEFERRED_DOWNLOAD;
                } else if(wifi_only_timer_expired_flag != 0) {
                    alarm = FUMO_RC_DOWNLOAD_FAILED_TIME_OUT;
                } else if(rc == MO_ERROR_DEVICE_FULL) {
                    // alert is already sent
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    break;
                } else if(gui_supported && download_by_user_cancel_flag) {
                    DM_LOGI("FUMO: download cancel by user");
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    alarm = MO_ERROR_INVALID_CREDENTIALS;
                    download_by_user_cancel_flag = 0;
                    remove_fw_package();
                } else if(gui_supported && !continue_download_via_lte
                        && !download_by_user_cancel_flag) {
                    show_gui_notification_download_fail();
                } else if (rc == PAL_RC_FRMW_DOWNLOAD_NO_NETWORK){
                    fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
                    break;
                }
            } else {
                alarm = FUMO_RC_SUCCESSFUL_DOWNLOAD;
                DM_LOGI("FUMO: download completed, event Download");
            }

            send_alarm_to_client(fta, alarm, alarm_type_download);
        }
        break;
        case fte_Update:
        {

            if (MO_ERROR_NONE != (rc = check_low_memory_status(UPDATE))) {
                break;
            }
            check_low_battery_status();

            fumo_storage_get_pkg_name(&pkg_name);
            if(update_cancel_flag &&( MO_ERROR_NONE!= pal_fumo_policy_check(pkg_name,PALFU_WORK_PATH)
                    || MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED!= pal_fumo_policy_check(pkg_name,PALFU_WORK_PATH))) {
                DM_LOGE("FUMO: update cancel; before; event Update");
                update_cancel_flag = 0;
                send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_UPDATE,
                    alarm_type_update);
                break;
            }

            DM_LOGI("FUMO: fumo_work_thread: event = fte_Update");
            g_update_state = 42; /*magic trash*/
            fumo_storage_set_state(FUMO_DEVICE_STATE_READY_TO_UPDATE);
            rc = update(fta);
            if(!rc) {
                DM_LOGI("FUMO: fumo_work_thread: Start waiting");
                /** @todo it hack, because we haven't event's */
                while( g_update_state == 42 ) {
                    sleep(1);
                }

                while( g_update_state == PAL_RC_FRMW_UPD_INPROGRESS) {
                    sleep(1);
                }
                alarm = FUMO_RC_SUCCESSFUL_UPDATE;
                DM_LOGI("FUMO: fumo_work_thread: End waiting");
                remove_fw_package();
            } else if (rc == FUMO_RC_PRV_SU_CANCEL_AFTER_UPDATE) {
                DM_LOGE("FUMO: update cancel; after; event Update");
                send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_NO_FIRMWARE, // 552
                    alarm_type_update);
                break;
            } else if (rc == FUMO_RC_PRV_SU_CANCEL_BEFORE_DEFERRED_EVENT) {
                DM_LOGE("FUMO: deferred update cancel; event Update");
                send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_DEFERRED_DOWNLOAD, // 553
                    alarm_type_update);
                break;
            } else if (rc == FUMO_RC_PRV_SU_CANCEL) {
                DM_LOGE("FUMO: update cancel; after; event Update");
                send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_UPDATE, // 554
                    alarm_type_update);
                break;
            } else if(gui_supported && install_by_user_cancel_flag) {
                install_by_user_cancel_flag = 0;
                alarm = MO_ERROR_OPERATION_CANCELED;
                DM_LOGI("FUMO: install cancel by user");
                remove_fw_package();
                fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                send_alarm_to_client(fta, alarm, alarm_type_update);
                break;
            } else if(rc == MO_ERROR_DEVICE_FULL) {
                // alert is already sent
                fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                break;
            } else if(rc == PAL_RC_FRMW_DOWNLOAD_ERROR_VERIFICATION_FAILED) {
                alarm = FUMO_RC_NOT_FOUND_OR_UPDATE_FAILED;
                remove_fw_package();
            } else {
                alarm = FUMO_RC_UPDATE_FAILED;
            }

            send_alarm_to_client(fta, alarm, alarm_type_update);

            if( PAL_RC_FRMW_UPD_COMPLETED_FAILED == g_update_state ||
                PAL_RC_FRMW_UPD_CANCELLED == g_update_state)
                fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_FAILED);
        }
        break;
        case fte_DownloadAndUpdate:
        {
			DM_LOGI("fumo_work_thread downloadandupdate start\n");
			rc = check_update_result(fta);  //if there is download or update
			if (rc) {
				fumo_cleanup();
				break;
			}
            check_low_battery_status();
            create_download_flag();
            {
                destroy_message_func(FUMO_DEVICE_STATE_UPDATE_COMPLETE_DATA);
                remove_complete_ui_flag();
            }
            int rc = download(fta);
            remove_download_flag();
            DM_LOGI("FUMO: rc=%d",rc);
            if(rc) {
                fumo_storage_set_state(FUMO_DEVICE_STATE_DOWNLOAD_FAILED);
                alarm = MO_ERROR_COMMAND_FAILED;
                if(rc == PAL_RC_FRMW_DOWNLOAD_ERROR_VERIFICATION_FAILED) {
                    if(gui_supported)
					{
						if (severity == NULL || severity_optional == atoi(severity))  //modified by chapin
							show_gui_notification_download_fail();
                    }
					alarm = FUMO_RC_NOT_FOUND_OR_UPDATE_FAILED;
                    remove_fw_package();
                    remove_network_up_flag();
                	fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_FAILED); //modified by chapin, verify failed, it will not download again
                } else if (rc == PAL_RC_FRMW_DOWNLOAD_ERROR_MEMORY) {
					alarm = FUMO_RC_DOWNLOAD_FAILED_NO_MEM;
                    if(gui_supported)
                        show_gui_notification_download_fail();

				} else if(rc == PAL_RC_FRMW_DOWNLOAD_SERVER_UNAVAILABLE) {
                    alarm = FUMO_RC_DL_SERVER_IS_UNAVAILABLE;
                    if(gui_supported)
                        show_gui_notification_download_fail();
                } else if(rc == MO_ERROR_DEVICE_FULL) {
                    // alert is already sent
                    remove_network_up_flag();
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    break;
                } else if(rc == PAL_RC_FRMW_DOWNLOAD_PACKAGE_NOT_ACCEPTABLE) {
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    alarm = FUMO_RC_UPDATE_PACKAGE_NOT_ACCEPTABLE;
                } else if(rc == FUMO_RC_PRV_SU_CANCEL) {
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    remove_fw_package();
                    remove_network_up_flag();
                    alarm = FUMO_RC_SU_CANCEL_SUCCESS_UPDATE;
                    DM_LOGE("FUMO: download cancel; before; event DownloadAndUpdate");
                    send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_UPDATE,
                        alarm_type_downloadandupdate);
                    break;
                } else if (rc == FUMO_RC_PRV_SU_CANCEL_BEFORE_DEFERRED_EVENT) {
                    DM_LOGE("FUMO: deferred download cancel; before; event DownloadAndUpdate");
                    send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_DEFERRED_DOWNLOAD,
                        alarm_type_downloadandupdate);
                    break;
                } else if (rc == PAL_RC_FRMW_DOWNLOAD_WIFI_TIMEOUT) {
                    remove_network_up_flag();
                    destroy_message_func(FUMO_DEVICE_STATE_DOWNLOAD_PROGRESSING);
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    alarm = FUMO_RC_DOWNLOAD_FAILED_TIME_OUT;
                } else if (wifi_only_timer_expired_flag != 0) {
                    alarm = FUMO_RC_DOWNLOAD_FAILED_TIME_OUT;
                } else if(gui_supported && download_by_user_cancel_flag) {
                    DM_LOGI("FUMO: download cancel by user");
                    fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                    alarm = MO_ERROR_INVALID_CREDENTIALS;
                    download_by_user_cancel_flag = 0;
                    remove_network_up_flag();
                    remove_fw_package();
                } else if(gui_supported && !continue_download_via_lte
                    && !download_by_user_cancel_flag) {
                    show_gui_notification_download_fail();
                }

                send_alarm_to_client(fta, alarm, alarm_type_downloadandupdate);
                break;
            } else
			{
                remove_network_up_flag();
                alarm = FUMO_RC_SUCCESSFUL_DOWNLOAD;
                DM_LOGI("FUMO: download completed, event Download");
                send_alarm_to_client(fta, alarm, alarm_type_downloadandupdate);
			}

            if (MO_ERROR_NONE != (rc = check_low_memory_status(UPDATE))) {
                break;
            }
            check_low_battery_status();

            DM_LOGI("FUMO: fte_DownloadAndUpdate: download completed");
            fumo_storage_set_state(FUMO_DEVICE_STATE_READY_TO_UPDATE);

            g_update_state = 42; /*magic trash*/
            rc = update(fta);
            if(!rc) {
                /** @todo it hack, because we haven't event's */
                while( g_update_state == 42 ) {
                    sleep(1);
                    DM_LOGI("FUMO: fte_DownloadAndUpdate: update g_update_state == 42");
                }

                while( g_update_state == PAL_RC_FRMW_UPD_INPROGRESS) {
                    sleep(1);
                    DM_LOGI("FUMO: fte_DownloadAndUpdate: update PAL_RC_FRMW_UPD_INPROGRESS ");
                }
                alarm = FUMO_RC_SUCCESSFUL_UPDATE;
                remove_fw_package();
            } else if (rc == FUMO_RC_PRV_SU_CANCEL_AFTER_UPDATE) {
                DM_LOGE("FUMO: update cancel; after; event DownloadAndUpdate");
                send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_NO_FIRMWARE, // 552
                    alarm_type_downloadandupdate);
                break;
            } else if (rc == FUMO_RC_PRV_SU_CANCEL_BEFORE_DEFERRED_EVENT) {
                DM_LOGE("FUMO: deferred update cancelled during DownloadAndUpdate event handling");
                send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_DEFERRED_DOWNLOAD, // 553
                    alarm_type_downloadandupdate);
                break;
            } else if (rc == FUMO_RC_PRV_SU_CANCEL) {
                DM_LOGE("FUMO: update cancel; before; event DownloadAndUpdate");
                send_su_cancel_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_UPDATE, // 554
                    alarm_type_downloadandupdate);
                break;
            } else if(gui_supported && install_by_user_cancel_flag) {
                install_by_user_cancel_flag = 0;
                alarm = MO_ERROR_OPERATION_CANCELED;
                DM_LOGI("FUMO: install cancel by user");
                remove_fw_package();
                fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                send_alarm_to_client(fta, alarm, alarm_type_update);
                break;
            } else if(rc == MO_ERROR_DEVICE_FULL) {
                // alert is already sent
                fumo_storage_set_state(FUMO_DEVICE_STATE_IDLE);
                break;
            } else if(rc == PAL_RC_FRMW_DOWNLOAD_ERROR_VERIFICATION_FAILED) {
                alarm = FUMO_RC_NOT_FOUND_OR_UPDATE_FAILED;
                remove_fw_package();
                fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_FAILED); //modified by chapin, verify failed, it will not download again
            } else {
                alarm = FUMO_RC_UPDATE_FAILED;
            }

            DM_LOGI("FUMO: fte_DownloadAndUpdate: update completed");
            if( PAL_RC_FRMW_UPD_COMPLETED_FAILED == g_update_state ||
                PAL_RC_FRMW_UPD_CANCELLED == g_update_state)
                fumo_storage_set_state(FUMO_DEVICE_STATE_UPDATE_FAILED);

            send_alarm_to_client(fta, alarm, alarm_type_downloadandupdate);
        }
        break;
        case fte_Cancel:
            send_alarm_to_client(fta, FUMO_RC_SU_CANCEL_SUCCESS_NO_FIRMWARE, alarm_type_downloadandupdate);
            DM_LOGI("FUMO: fte_Cancel");
        break;
        default:
            g_fumo_work_thread_id = 0;
            return 0;
    }

    //writing history file
    //if(gui_supported)  //modified by chapin
     // update_history(alarm);

    free(args);
    g_fumo_work_thread_id = 0;
    download_cancel_flag = 0;
    update_cancel_flag = 0;
	DM_LOGI("FUMO: fumo work thread end");
    return (void*)rc;
};

/**
    @brief System Update History, create, update file
    @param[out]
    @return RESULT_SUCCESS
*/
int update_history(int error_code)
{
    int rc = 0;
    char cur_date[40] = "\0";
    char cur_time[40] = "\0";
    time_t rawtime = 0;
    struct tm * ptm = NULL;

    char* software_version = NULL;
    char* text_message = NULL;
    char* status_message = NULL;
    char* latest_system_update_date = NULL;
    char* latest_system_update_time = NULL;
    char* learn_more_url = NULL;
    char* failed_error = NULL;

    char* path = MO_WORK_PATH"/data/updateInfo.json";

    /** get date and time */
    time(&rawtime);
    ptm = gmtime(&rawtime);
    strftime(cur_date, sizeof(cur_date), "%m:%d:%Y", ptm);
    strftime(cur_time, sizeof(cur_time), "%H:%M", ptm);
    latest_system_update_date = cur_date;
    latest_system_update_time = cur_time;

    /** get version */
    if(MO_ERROR_NONE != (rc = fumo_storage_get_string_value(offsetof(fumo_storage_t, pkg_version),
            PATH_MAX, &software_version))) {
        DM_LOGI("FUMO: UPDATE_HISTORY get pkg_version error = %d", rc);
    }

    /** post update url for learn more online */
    if(MO_ERROR_NONE != (rc = fumo_storage_get_string_value(offsetof(fumo_storage_t, post_update_url),
            PATH_MAX, &learn_more_url))) {
        DM_LOGI("FUMO: UPDATE_HISTORY get post_update_url error = %d", rc);
    }

    /** post update message for text message */
    if(MO_ERROR_NONE != (rc = fumo_storage_get_string_value(offsetof(fumo_storage_t, post_update_message),
            PATH_MAX, &text_message))) {
         DM_LOGI("FUMO: UPDATE_HISTORY get post_update_message error = %d", rc);
    }

    if(error_code == FUMO_RC_SUCCESSFUL_UPDATE) {
        status_message = "Success";
    } else {
        status_message = "Failed";
    }

    struct jsonObject swv;
    struct jsonObject updateStatus;
    struct jsonObject text;
    struct jsonObject date;
    struct jsonObject time;
    struct jsonObject url;
    struct jsonObject error;

    char *fullTempJSO = NULL;

    if(software_version != NULL) {
        set(&swv,"Software version", software_version);
        jsonWrapper(&fullTempJSO, &swv);
    }
	DM_LOGI("FUMO: set update_time\n");

    if(status_message != NULL) {
        set(&updateStatus,"Update status", status_message);
        jsonWrapper(&fullTempJSO, &updateStatus);
    }

    if(text_message != NULL) {
        set(&text,"Text", text_message);
        jsonWrapper(&fullTempJSO, &text);
    }

    if(latest_system_update_date != NULL) {
        set(&date,"Date", latest_system_update_date);
        jsonWrapper(&fullTempJSO, &date);
    }

	DM_LOGI("FUMO: set update_time\n");
    if(latest_system_update_time != NULL) {
        set(&time,"Time", latest_system_update_time);
        jsonWrapper(&fullTempJSO, &time);
    }

    if(learn_more_url != NULL) {
        set(&url,"learn more URL", learn_more_url);
        jsonWrapper(&fullTempJSO, &url);
    }

	DM_LOGI("FUMO: set failed code\n");
    failed_error = (char*) calloc(32 , sizeof(char));
	memset(failed_error, 0, 32);
	DM_LOGI("FUMO: set failed code end\n");
    sprintf(failed_error, "%d", error_code);
    if(failed_error != NULL) {
        set(&error,"failure error", failed_error);
        jsonWrapper(&fullTempJSO, &error);
    }
	DM_LOGI("FUMO: finish json object\n");

    finishJsonObject(&fullTempJSO);

    DM_LOGI("FUMO: UPDATE_HISTORY fullTempJSO = %s", fullTempJSO);

    int result = saveJsonObjectToFile(path, fullTempJSO);

    return result;
}

/**
    @brief Checks available space on the device
    @param[out] need_space Size (in bytes) of the file which
    we will download
    @return MO_ERROR_SUCCESS if success
            MO_ERROR_DEVICE_FULL if there is not free memory
*/
int check_avail_space(unsigned long need_space)
{
    void* pal_handle_memcheck = NULL;
    if (NULL == pal_handle_memcheck && NULL == (pal_handle_memcheck=dlopen(
                    PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY))) {
        DM_LOGE("FUMO: Can't open library: %s",
                PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        return RESULT_ERROR;
    }

    unsigned long long bytes = 0;
    int (* pal_system_memory_free_by_path)(const char*, unsigned long long*)
            = dlsym(pal_handle_memcheck, "pal_system_get_memory_free_by_path");
    pal_system_memory_free_by_path(CURRENT_DATA_LOCATION, &bytes);
    dlclose(pal_handle_memcheck);
	DM_LOGE("check_avail_space need_space = %lu, bytes = %lu\n", need_space, (unsigned long)(bytes / 1024));
    if (need_space > bytes) {
        // Store available space in bytes for executing system update
        fumo_storage_set_long_value(offsetof(fumo_storage_t, required_space_for_update), need_space);
        // Store space in bytes which it is needed to delete for executing system update
        fumo_storage_set_long_value(offsetof(fumo_storage_t, required_space_for_delete), need_space - (unsigned long)bytes);
        return MO_ERROR_DEVICE_FULL;
    } else {
        return MO_ERROR_SUCCESS;
    }
}

int get_battery_level(int* battery_status)
{
    void *pal_handle = NULL;
    int  get_battery_err = 0;
    float battery_level = 0;
    int32_t p_value = 0;
    int error_code = 0;
    int i = 0;

    if(NULL == pal_handle && NULL == (pal_handle=dlopen(
                        PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY))) {
        DM_LOGE("FUMO: Can't open library: %s",
            PAL_INSTALL_DIR "/" PAL_LIB_NAME);
        return RESULT_ERROR;
    }

    /*Getting count of batteries*/
    battery_count_get_func = dlsym(pal_handle, "pal_system_batteries_get");
    get_battery_err = battery_count_get_func(&p_value);
	DM_LOGI("get_battery_level get_battery_err = %d\n", get_battery_err);

    switch(get_battery_err)
    {
        case RESULT_ERROR_INVALID_STATE:
            *battery_status = 1;
            error_code = RESULT_SUCCESS;
            break;

        case RESULT_SUCCESS:
            for (i = 0; i < p_value; i++) {
                battery_level_get_func = dlsym(pal_handle, "pal_system_battery_x_level_get");
                if (RESULT_SUCCESS == battery_level_get_func(i, &battery_level)) {
                    if ( (*battery_status == 0) && (battery_level >= FUMO_BATTERY_LEVEL) ) {
                        DM_LOGI("FUMO: Battery level = %f", battery_level);
                        *battery_status = 1;
                        error_code= RESULT_SUCCESS;
                        break;
                    }
                } else {
                    error_code = RESULT_ERROR;
                }
            }
            break;

        default:
            error_code = get_battery_err;
    }

    return error_code;
}

int get_required_size (unsigned long* download_size, unsigned long* install_size)
{
    unsigned long size = 0;
    unsigned long required_install_parametr = 0;

    const int d_free_space_multiplier = 4;//download
    const int u_free_space_multiplier = 3;//update

    /** Get required size from storage*/
    if (MO_ERROR_NONE != fumo_storage_get_long_value(
            offsetof(fumo_storage_t, required_install_parametr), &required_install_parametr)) {
        *install_size = (*download_size) * u_free_space_multiplier;
    }

    if (MO_ERROR_NONE != fumo_storage_get_long_value(
            offsetof(fumo_storage_t, size), &size)) {
        *download_size = FUMO_DOWNLOAD_FILE_SIZE;
    }

    if(size == 0 || required_install_parametr == 0) {
        *download_size = FUMO_DOWNLOAD_FILE_SIZE;
        *install_size = (*download_size) * u_free_space_multiplier;
    } else {
        *download_size = size;
        *install_size = required_install_parametr;
    }
    DM_LOGI("FUMO: DDDownload before size %lu",*download_size);

	*install_size = 0;  //modified by chapin
	*download_size *= d_free_space_multiplier;
	//*download_size = 400 * 1024 * 1024;  

    DM_LOGI("FUMO: DDDownload size %lu",*download_size);
    DM_LOGI("FUMO: IIInstall size %lu",*install_size);

    return RESULT_SUCCESS;
}

void update_button_wifi_timer_callback()
{
    DM_LOGI("FUMO: update_button_wifi_timer_callback");
    fumo_storage_set_int_value(offsetof(fumo_storage_t, wifi_only), 0);
    gui_response.button_id = ebt_check_for_update;
    pthread_mutex_lock(&wifi_wait_mutex);
    pthread_cond_signal(&wifi_wait_conditional);
    pthread_mutex_unlock(&wifi_wait_mutex);
}

void update_button_defer_timer_callback()
{
    DM_LOGI("FUMO: update_button_defer_timer_callback");
    gui_response.button_id = ebt_check_for_update;
    pthread_mutex_lock(&defer_wait_mutex);
    pthread_cond_signal(&defer_wait_conditional);
    pthread_mutex_unlock(&defer_wait_mutex);
}

void get_update_button_result(void* callback)
{
    pal_register_callback_t register_callback = NULL;
    if(NULL == (register_callback =
        (pal_register_callback_t)get_pal_method("pal_register_omadm_callback")))
    {
        DM_LOGI("FUMO: check_for_update_pressed MO_ERROR_COMMAND_FAILED");
        return;
    }
    omadmCallback button_callback = callback;
    int status = register_callback(OMADM_FUMO_CHECK_FOR_UPDATE, button_callback);
    if (status == RESULT_SUCCESS) DM_LOGI("FUMO: get_update_button_result success");
}

/**
 * init timer for wifiOnly timer
 * @return id, 0 if timer inited, -1 if error
 */
int wifi_only_timer_init()
{
    struct timespec ts_wifi;
    int wifi_only_timer;
    int rc = 0;
    /** Get WiFiOnlyTimer */
    if(MO_ERROR_NONE != fumo_storage_get_int_value(
            offsetof(fumo_storage_t, wifi_only_timer), &wifi_only_timer)) {
        DM_LOGE("FUMO: Can not get wifi_only_timer from fumo_state_storage");
        return -1;
    }

    int wifi_only_timer_sec = wifi_only_timer*3600;

    clock_gettime(CLOCK_REALTIME, &ts_wifi);
    ts_wifi.tv_sec += wifi_only_timer_sec;
    ts_wifi.tv_nsec = 0;
    fumo_storage_set_time_value(
                offsetof(fumo_storage_t, wifi_only_check_timer), ts_wifi.tv_sec);
    DM_LOGI("FUMO: init for wifi only timer ");
    return rc;
}

/**
 * Wait WiFi connected or expired WiFiOnly timer
 * @return id, 0 if WiFi connected, not 0 otherwice
 */
int wait_wifi()
{
    int rc = MO_ERROR_NONE;
    __time_t saved_expired_time;

    wifi_only_timer_init();

    if (MO_ERROR_NONE != fumo_storage_get_time_value(offsetof(fumo_storage_t, wifi_only_check_timer),
            &saved_expired_time)) {
        DM_LOGE("FUMO: Can not get wifi_only_check_timer from fumo_state_storage");
        return MO_ERROR_COMMAND_FAILED;
    }

    struct timespec ts_wifi;
    clock_gettime(CLOCK_REALTIME, &ts_wifi);
    ts_wifi.tv_sec = saved_expired_time;
    ts_wifi.tv_nsec = 0;

    DM_LOGI("FUMO: Waiting for Wi-Fi connected ...");
    pthread_mutex_lock(&wifi_wait_mutex);
    get_update_button_result(update_button_wifi_timer_callback);
    rc = pthread_cond_timedwait(&wifi_wait_conditional, &wifi_wait_mutex, &ts_wifi);
    pthread_mutex_unlock(&wifi_wait_mutex);
    return rc;
}

/**
 * Wait cancel process or expired timer for deferred process.
 * @param[in] second - user defined real time.
 * @return id, 0 cancel, not null - expired timer
 */
int wait_defer_timer(unsigned long seconds)
{
    int rc = 0;

    struct timespec ts_defer;
    ts_defer.tv_sec = seconds;
    ts_defer.tv_nsec = 0;
    DM_LOGE("FUMO: ts_defer.tv_sec = %ld", ts_defer.tv_sec);
    if(MO_ERROR_NONE != (rc=fumo_storage_set_long_value(
            offsetof(fumo_storage_t, defer_time), ts_defer.tv_sec))) {
        DM_LOGE("FUMO: set defer_time rc = %d", rc);
        return rc;
    }

    DM_LOGI("FUMO: Waiting for defer timer expired ... %ld", ts_defer.tv_sec);
    pthread_mutex_lock(&defer_wait_mutex);
    get_update_button_result(update_button_defer_timer_callback);
    rc = pthread_cond_timedwait(&defer_wait_conditional, &defer_wait_mutex, &ts_defer);
    pthread_mutex_unlock(&defer_wait_mutex);

    DM_LOGI("FUMO: wait_defer_timer exit rc = %d", rc);
    return rc;
}

/**
 * Wait for user reaction
 * @return id of button which has been pressed
 */
int wait_for_user_reaction()
{
    DM_LOGI("FUMO: Waiting for user reaction...");
    pthread_mutex_lock(&gui_wait_mutex);
    pthread_cond_wait(&gui_wait_conditional, &gui_wait_mutex);
    pthread_mutex_unlock(&gui_wait_mutex);
    DM_LOGI("FUMO: User pressed button #%d", gui_response.button_id);
    return gui_response.button_id;
}

/**
 * Wait for user reaction or timeout
 * @param seconds timeout in seconds
 * @return id of button which has been pressed or 0 if timeout expired
 */
int wait_for_user_reaction_or_timer(int seconds)
{
    int rc;
    DM_LOGI("FFFFFUMO: Waiting for user reaction or timer...");
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += seconds;
    ts.tv_nsec = 0;
    pthread_mutex_lock(&gui_wait_mutex);
    rc = pthread_cond_timedwait(&gui_wait_conditional, &gui_wait_mutex, &ts);
    pthread_mutex_unlock(&gui_wait_mutex);
    if (rc == 0) {
        DM_LOGI("FUMO: User pressed button #%d", gui_response.button_id);
        return gui_response.button_id;
    } else {
        g_key_deferred = 0;
        DM_LOGI("FUMO: wait_for_user_reaction_or_timer: Timeout expired");
        return 0;
    }
}

/**
 * Display dialog "Update Error"
 * @param make maker of device
 * @param model model of device
 */
void show_update_error_dialog(char *make, char *model)
{
    if(gui_supported) {
    #ifdef ANDROID
        ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
        pal_fumo_gui_message_descriptor * message_descriptor = NULL;
        if(NULL == (gui_create_message_func =
                        (ft_pal_fumo_gui_create_message)get_pal_method("pal_fumo_gui_create_message")))
            return;
        // Show "System update available" dialog
        if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
                        calloc(1, sizeof(pal_fumo_gui_message_descriptor)))) {
            DM_LOGE("FUMO: node_command_exec: Out of memory");
            return;
        }
        memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
        message_descriptor->state = FUMO_DEVICE_STATE_UPDATE_FAILED;
        message_descriptor->message_type = emt_pop_up;
        message_descriptor->message_mode = emm_persistent;
        message_descriptor->icon_type = eit_device;
        message_descriptor->header_text = strdup("System update unsuccessful");
        //  @todo OOM check
        if (NULL == make || NULL == model) {
            message_descriptor->message_text = strdup(
                    "Sorry, there was a problem updating your device. No changes were made.");
        } else {
            char *str_begin = "Sorry, there was a problem updating your ";
            char *str_end = ". No changes were made.";
            int len = strlen(str_begin) + strlen(make) + strlen(" ")
                    + strlen(model) + strlen(str_end) + 1;
            char str[len];
            str[0] = '\0';
            strcat(str, str_begin);
            strcat(str, make);
            strcat(str, " ");
            strcat(str, model);
            strcat(str, str_end);
            message_descriptor->message_text = strdup(str);
        }
        //  @todo OOM check
        message_descriptor->hyper_link_caption = strdup("");
        //  @todo OOM check
        message_descriptor->hyper_link = strdup("");
        //  @todo OOM check
        message_descriptor->progress_bar_caption = strdup("");
        message_descriptor->button_type = ebt_yes;
        message_descriptor->user_reaction = NULL;

        DM_LOGI("FUMO: Show dialog <System Update Unsuccessful>");
        gui_create_message_func(message_descriptor);
        clean_gui_message_descriptor(message_descriptor);
#endif
    }
}

/**
 * Display dialog "Update complete"
 * @param make maker of device
 * @param model model of device
 */
void show_update_complete_dialog()
{
    if(gui_supported) {
#ifdef ANDROID
        ft_pal_fumo_gui_create_message gui_create_message_func = NULL;
        pal_fumo_gui_message_descriptor * message_descriptor = NULL;
        if(NULL == (gui_create_message_func =
                        (ft_pal_fumo_gui_create_message)get_pal_method("pal_fumo_gui_create_message")))
            return;
        // Show "System update complete" dialog
        if (NULL == (message_descriptor = (pal_fumo_gui_message_descriptor*)
                        calloc(1, sizeof(pal_fumo_gui_message_descriptor)))) {
            DM_LOGE("FUMO: node_command_exec: Out of memory");
            return;
        }
        memset(message_descriptor, 0, sizeof(pal_fumo_gui_message_descriptor));
        message_descriptor->state = FUMO_DEVICE_STATE_UPDATE_COMPLETE_DATA;
        message_descriptor->message_type = emt_pop_up;
        message_descriptor->message_mode = emm_persistent;
        message_descriptor->icon_type = eit_device;
        //  @todo OOM check
        message_descriptor->header_text = strdup("System Update Complete");
        if (MO_ERROR_NONE != fumo_storage_get_string_value(
                offsetof(fumo_storage_t, post_update_message),
                PATH_MAX, &(message_descriptor->message_text))) {
            clean_gui_message_descriptor(message_descriptor);
            free(message_descriptor);
            return;
        }
        if (MO_ERROR_NONE == fumo_storage_get_string_value(
                        offsetof(fumo_storage_t, post_update_url),
                        PATH_MAX, &(message_descriptor->hyper_link))){
            //  @todo OOM check
            message_descriptor->hyper_link_caption = strdup("Learn More");
        } else {
            //  @todo OOM check
            message_descriptor->hyper_link = strdup("");
            //  @todo OOM check
            message_descriptor->hyper_link_caption = strdup("");
        }
        //  @todo OOM check
        message_descriptor->progress_bar_caption = strdup("");
        message_descriptor->button_type = ebt_yes;
        message_descriptor->user_reaction = NULL;

        DM_LOGI("FUMO: Show dialog <System Update Complete>");
        gui_create_message_func(message_descriptor);
        clean_gui_message_descriptor(message_descriptor);
#endif
    }
}
/**
 * Remove firmware package with a download descriptor
 */
void remove_fw_package()
{
    ft_create_download_descriptor create_download_descriptor_func = NULL;
    ft_free_download_descriptor free_download_descriptor_func = NULL;
    DM_LOGI("FUMO: remove_fw_package");
    if (NULL == (create_download_descriptor_func =
                    (ft_create_download_descriptor) get_pal_method(
                            "pal_create_download_descriptor")))
        return;
    if (NULL == (free_download_descriptor_func =
                    (ft_free_download_descriptor) get_pal_method(
                            "free_download_descriptor_t")))
        return;

    pal_download_descriptor_t *download_descriptor = NULL;
    if (PAL_RC_FRMW_DOWNLOAD_SUCCESS
            == create_download_descriptor_func(NULL, &download_descriptor)) {
        DM_LOGI("FUMO: removing firmware package");
        char path[PATH_MAX];
        path[0] = 0;
        strcat(path, CURRENT_DATA_LOCATION "/");
        strcat(path, download_descriptor->packageName);
        remove(path);//remove firmware package
        free_download_descriptor_func(&download_descriptor);
        remove(CURRENT_DATA_LOCATION "/" DOWNLOAD_DESCRIPTOR_FILE_NAME);//remove descriptor
    }
}

/**
 * Get status if Wi-Fi network
 * @return 0 if not connected
 *         1 if connected
 */
static int get_wifi_state()
{
    int status = -1;
    int state = -1;
    if (RESULT_SUCCESS != wifi_status_get(&status))
        DM_LOGE("FUMO: pal_network_wifi_status_get() error");
    if (RESULT_SUCCESS != wifi_state_get(&state))
        DM_LOGE("FUMO: pal_network_wifi_state_get() error");
    DM_LOGI("FUMO: wifi status = %d state = %d", status, state);
    if (0 != state)// completed and enabled
        return 1;
    else
        return 0;
}

/**
    @brief  Checking if battery has enough level for continue system update process
*/
void check_low_battery_status()
{
    int battery_status = 0;
    int alarm = MO_ERROR_COMMAND_FAILED;
    ft_pal_fumo_gui_destroy_message destroy_message_func = NULL;
    if(NULL == (destroy_message_func = get_pal_method("pal_fumo_gui_destroy_message"))) {
        return;
    }
    /*if ((get_battery_level(&battery_status) == RESULT_SUCCESS) && (battery_status == 0)) {
        DM_LOGI("FUMO: battery is low");
        fumo_storage_set_state(FUMO_DEVICE_STATE_BATTERY_WAITING);
        alarm = FUMO_RC_UPDATE_DEFERRED;
        send_alarm_to_client(fta, alarm, alarm_type_download);
        if(gui_supported) {
            show_low_batt_notification();
        }
        do {
            DM_LOGI("FUMO: Waiting for the sufficient battery level");
            sleep(60);
        } while((get_battery_level(&battery_status) == RESULT_SUCCESS) && (battery_status == 0));
            destroy_message_func(FUMO_DEVICE_STATE_LOW_BATTERY);
    }*/
    DM_LOGI("FUMO: Battery level: OK");
}

int check_update_result(fumo_thread_args *fta)
{
	int state = FUMO_DEVICE_STATE_IDLE;
	FILE *file = NULL;
	int count = -1;
	int alarm = MO_ERROR_COMMAND_FAILED;
	fumo_storage_get_state(&state);
	DM_LOGI(" check_update_result state = %d\n", state);
	if (state != FUMO_DEVICE_STATE_UPDATE_RESTART)
		return 0;

	DM_LOGI("check_update_result state = %d\n", state);
	if(0 == access(RECOVERY_SUCCESS_FLAG_FILE_PATH, F_OK)) {
		alarm = FUMO_RC_SUCCESSFUL_UPDATE;
		show_update_complete_dialog();
		remove(RECOVERY_SUCCESS_FLAG_FILE_PATH);
		DM_LOGI("check_update success\n");
    } else if (0 == access(RECOVERY_FAIL_FLAG_FILE_PATH, F_OK)) {
		alarm = FUMO_RC_UPDATE_FAILED;  //update set 402
		show_update_error_dialog(NULL, NULL);
		remove(RECOVERY_FAIL_FLAG_FILE_PATH);
		DM_LOGI("check_update fail\n");
	}else {
		DM_LOGI("there is no recovery flag in /cache folder");
		return 0;
    }

	remove_fw_package();
	DM_LOGI("check_update_result send alert\n");
	send_alarm_to_client(fta, alarm, alarm_type_downloadandupdate);
    if(gui_supported)  //modified by chapin
      update_history(alarm);

	return 1;
}

int create_download_flag()
{
	FILE *file = NULL;
	file = fopen(DOWNLOAD_FLAG_FILE_PATH, "w+");
	DM_LOGI("createeee download %s\n", DOWNLOAD_FLAG_FILE_PATH);
	if (file == NULL)
	{
		DM_LOGE(" open file fail errno = %d\n", errno);
		return -1;
	}

	fwrite("1", 1, 1, file);
	fclose(file);
	return 0;
}

int remove_download_flag()
{
	DM_LOGI("remove download flag = %s\n", DOWNLOAD_FLAG_FILE_PATH);
	remove(DOWNLOAD_FLAG_FILE_PATH);
	if (0 != access(DOWNLOAD_FLAG_FILE_PATH, F_OK))
		DM_LOGI("download file wat remove\n");
	return 0;
}

int create_network_up_flag()
{
	FILE *file = NULL;
	file = fopen(NETWORK_UP_FLAG_FILE_PATH, "w+");
	DM_LOGI("create network up flag %s\n", NETWORK_UP_FLAG_FILE_PATH);
	if (file == NULL)
	{
		DM_LOGE(" open file fail errno = %d\n", errno);
		return -1;
	}

	fwrite("1", 1, 1, file);
	fclose(file);
	return 0;
}

int remove_network_up_flag()
{
	DM_LOGI("remove network flag = %s\n", NETWORK_UP_FLAG_FILE_PATH);
	remove(NETWORK_UP_FLAG_FILE_PATH);
	if (0 != access(NETWORK_UP_FLAG_FILE_PATH, F_OK))
		DM_LOGI("network up file wat remove\n");
	return 0;
}

int remove_complete_ui_flag()
{
	DM_LOGI("remove complete ui flag = %s\n", NOTIFICATION_UI_FLAG_FILE_PATH);
	remove(NOTIFICATION_UI_FLAG_FILE_PATH);
	if (0 != access(NOTIFICATION_UI_FLAG_FILE_PATH, F_OK))
		DM_LOGI("network up file wat remove\n");
	return 0;
}


/**
    @brief  Checking if memory is enough for continue system update process
*/
int check_low_memory_status(fumo_process_type process_type)
{
    int ret_val = MO_ERROR_NONE;
    unsigned long download_size = 0;
    unsigned long install_size = 0;
    get_required_size(&download_size, &install_size);
    unsigned long required_size = process_type == DOWNLOAD ? download_size : install_size;
    int failed_state = process_type == DOWNLOAD ? FUMO_DEVICE_STATE_DOWNLOAD_FAILED :
            FUMO_DEVICE_STATE_UPDATE_FAILED;
    int alarm = FUMO_RC_DOWNLOAD_FAILED_NO_MEM;
    const char* alarm_type = process_type == DOWNLOAD ? alarm_type_download : alarm_type_update;
    DM_LOGI("FUMO: check_low_memory_status used\n");

    if (check_avail_space(required_size) != MO_ERROR_SUCCESS) {
        DM_LOGI("FUMO: Not enough available space");
        fumo_storage_set_state(failed_state);

#ifndef ANDROID
        send_alarm_to_client(fta, alarm, alarm_type);
        remove_fw_package();
        return MO_ERROR_DEVICE_FULL;
    }
#else
        char *severity = NULL;
        if(MO_ERROR_NONE != fumo_storage_get_string_value(
                offsetof(fumo_storage_t, severity), NAME_MAX, &severity)) {
            DM_LOGE("FUMO: Can not get severity from fumo_state_storage");
            return MO_ERROR_COMMAND_FAILED;
        }

        ft_pal_fumo_gui_destroy_message destroy_message_func = NULL;
        destroy_message_func = (ft_pal_fumo_gui_destroy_message)
                get_pal_method( "pal_fumo_gui_destroy_message");
        if(destroy_message_func == NULL) {
            return MO_ERROR_COMMAND_FAILED;
        }

        int rc = 0;
        int timer = 0;
        int not_now_button_counter = 0;
        do {

            if ((rc != 0 && rc != ebt_no) || (rc == 0 && timer == 0)) {
                show_gui_dialog_check_memory();
            }

            if ((rc = wait_for_user_reaction_or_timer(DELAY_5_MINUTES)) == 0) {
                timer += DELAY_5_MINUTES;
                if (timer < DELAY_24_HOURS) {
                    continue;
                } else {
                    timer = 0;
                }
            } else {
                if(ebt_no == gui_response.button_id) {
                    not_now_button_counter++;
                    if(not_now_button_counter == 6) {
                        //if((process_type == DOWNLOAD) && (severity_optional == atoi(severity))) {
                        if(process_type == DOWNLOAD) //modified by chapin
                        {
						    send_alarm_to_client(fta, alarm, alarm_type);
                            remove_fw_package();
                            ret_val = MO_ERROR_DEVICE_FULL;
                        }
                        break;
                    }
                }
            }
        } while (check_avail_space(required_size) != MO_ERROR_SUCCESS);
        destroy_message_func(FUMO_DEVICE_STATE_MEMORY_NOT_ENOUGH);
    }
#endif
    DM_LOGI("FUMO: Available space: OK");
    return ret_val;
}
