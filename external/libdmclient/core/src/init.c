/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "controller.h"
#include "dm_common.h"
#include "dm_logger.h"
#include "plugin_manager.h"

#include "pal.h"
#include "net_manager.h"
#include "init.h"

extern int omadm_event_handler_init();
extern bool omadm_controller_init();
extern int omadm_controller_restart_session();
extern int omadm_controller_emulate_su_cancel();
extern int omadm_controller_user_update_session();
extern void omadm_controller_load_eternal_plugins();

#define AUTO_SESSION_FILE_NAME "startautosession"
#define SU_CANCEL_FILE_NAME "su_cancel"
#define MAX_AUTO_SESSION_ATTEMPTS 200
#define TIMEOUT_BETWEEN_SESSIONS 30 //in seconds
static bool running = false;

void stop_main_loop();


void handle_signal(int sig)
{
    if(sig == SIGINT) {
        stop_main_loop();
        /* Reset signal handling to default behavior */
        signal(SIGINT, SIG_DFL);
    } else if(sig == SIGHUP) {
        DM_LOGI("Debug: received SIGHUP signal");
    } else if (sig == SIGPIPE) {
        DM_LOGI("Debug: received SIGPIPE signal");
    }
}

void run_main_loop()
{
    DM_LOGI("Starting main loop. Current release version");
    FILE *file;
    int count = 0;
    running = true;
	char file_name_path[128];
	memset(file_name_path, 0, sizeof(file_name_path));

    // Load eternal plugins at start up from the plugins directory
    omadm_controller_load_eternal_plugins();
    while (running) {
        //TODO: main thread
        DM_LOGI("I'm alive! Current release version");
        // START AUTOSESSION

		sprintf(file_name_path, "%s/%s", CURRENT_DATA_LOCATION, AUTO_SESSION_FILE_NAME);
        if(0 != access(file_name_path, F_OK)) {
            //DM_LOGD("The [startautosession] file is not exist %s", CURRENT_DATA_LOCATION"/"AUTO_SESSION_FILE_NAME);
        } else {
            file=fopen(file_name_path, "r");
            if (file) {
                fscanf(file, "%d", &count);
                DM_LOGD("Session counter [%d]", count);
                if(count > MAX_AUTO_SESSION_ATTEMPTS) {
                    count = MAX_AUTO_SESSION_ATTEMPTS;
                }
                if(count > 0) {
                    netm_up_admin_network();
                    netm_is_network_ready_lock();
                    DM_LOGE("INIT: Network is ready");
                    omadm_controller_restart_session();
                    count--;
                }
                fclose(file);
            } else {
                DM_LOGD("Cannot open [startautosession] file for read");
            }
            file=fopen(file_name_path, "w");
            if (file) {
                fprintf(file, "%d", count);
                fclose(file);
            } else {
                DM_LOGD("Cannot open [startautosession] file for write");
            }
        }
        // EMULATE SU CANCEL
        if(0 != access(CURRENT_DATA_LOCATION"/"SU_CANCEL_FILE_NAME, F_OK)) {
            DM_LOGD("The [su_cancel] file is not exist");
        } else {
            omadm_controller_emulate_su_cancel();
            DM_LOGD("Removing [su_cancel] file");
            remove(CURRENT_DATA_LOCATION"/"SU_CANCEL_FILE_NAME);
        }

	 	// EMULATE SU CANCEL
        /*if(0 != access(RECOVERY_FLAG_FILE_PATH, F_OK)) {
            DM_LOGD("The [su_cancel] file is not exist");
        } else {
            omadm_controller_user_update_session();
            DM_LOGD("Removing [update_flag] file");
            remove(RECOVERY_FLAG_FILE_PATH);
        }*/

        sleep(TIMEOUT_BETWEEN_SESSIONS);
    }
}

void stop_main_loop()
{
    DM_LOGI("Stopping main loop");
    running = false;
}

int dmclient_init()
{
    /// \todo remove dbg string
    DM_OPEN_LOG("OMADM-DBG");

    signal(SIGINT, handle_signal);
    signal(SIGHUP, handle_signal);
    signal(SIGPIPE, handle_signal);
    sleep(60);  //sleep for the network is ok

    /* DMC CONTROLLER */
    if (!init_dmcController())
    {
        DM_LOGE("Error: DM Controller initialization FAILED! | in %s (line: %d)", __FILE__, __LINE__);
        return 1; /// \todo Do we need some specific error codes here?
    }
    // Do we need to initialize omadm&udm controllers right here? Or better to move it into controller?
    /// \todo initialize UDM_controller here

    /* PLUGIN MANAGER */
    if (!init_pluginManager())
    {
        DM_LOGE("Error: Plugin Manager initialization FAILED! | in %s (line: %d)", __FILE__, __LINE__);
        return 2; /// \todo Do we need to die here?
    }
    /* EVENT HANDLER */
    if (omadm_event_handler_init()) {
        DM_LOGE("Error: Event Handler initialization FAILED! | in %s (line: %d)", __FILE__, __LINE__);
        return 3;
    }
    /* NETWORK MANAGER */
    if(!netm_init_network_manager()) {
        DM_LOGE("Error: Netowrk Manager initialization FAILED! | in %s (line: %d)", __FILE__, __LINE__);
        return 4;
    }
    /* OMADM CONTROLLER */
    if(!omadm_controller_init()) {
        DM_LOGE("Error: Omadm Controller initialization FAILED! | in %s (line: %d)", __FILE__, __LINE__);
        return 5;
    }
 
	if (check_network_up_flag())  //if there is fail in download, it neeed to download again
	{
		netm_up_admin_network();
	}

    run_main_loop();
    /// \todo call release for OMADM & UDM controllers

    stop_pluginManager();
    stop_dmcController();
    netm_stop_network_manager();
    DM_LOGI("Daemon stopped");
    DM_CLOSE_LOG();

    return 0;
}

// Only for unit-testing purposes
bool test_main_loop_running()
{
    return running;
}
