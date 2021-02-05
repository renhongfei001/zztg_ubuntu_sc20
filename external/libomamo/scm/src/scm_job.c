/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "pal.h"
#include "pal_scomo.h"
#include "scm.h"

pthread_mutex_t download_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t downloadinstall_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t inactive_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Check conditions
 * @param battery battery level
 * @param memory memory amount
 * @return 0 if OK
 *         1 if not
 */
int scm_verify_conditions_ok(int battery, int memory)
{
    //TODO
    if (MIN_REQ_BATTERY > battery || MIN_REQ_MEMORY > memory) {
        return 1;
    } else {
        return 0;
    }
}

int mutex_init_all()
{
    int rc = 0;
    rc += pthread_mutex_init(&download_mutex, NULL);
    rc += pthread_mutex_init(&downloadinstall_mutex, NULL);
    rc += pthread_mutex_init(&inactive_mutex, NULL);
    return rc;
}

int mutex_destroy_all()
{
    int rc = 0;
    rc += pthread_mutex_destroy(&download_mutex);
    rc += pthread_mutex_destroy(&downloadinstall_mutex);
    rc += pthread_mutex_destroy(&inactive_mutex);
    return rc;
}

/**
 * Select first DOWNLOAD task from queue
 * @param [out]info information about task
 * @return 0 if task exists
 *         not 0 if there are not DOWNLOAD tasks in queue
 */
int scm_select_download_job(scm_job_description *info)
{
    int rc = 0;
    rc = scm_get_job_from_queue("Download", info);
    if (0 == rc &&
        SCM_JOB_WAS_PUSH_TO_QUEUE == info->state &&
        0 == pthread_mutex_trylock(&download_mutex)) {
        //TODO put some data below
        rc += scm_verify_conditions_ok(50, 100);
        if (0 == rc) { // verify OK
            scm_set_job_state_in_queue(SCM_JOB_IS_PROCESSING, info->job_key);
            rc += scm_download_package();
        }
        if (0 == rc) { // download OK
            scm_set_job_state_in_queue(SCM_JOB_IS_FINISHED_SUCCESS, info->job_key);
            scm_pop_job_from_queue(info->job_key);
        } else {
            scm_set_job_state_in_queue(SCM_JOB_IS_FAILED, info->job_key);
        }

        pthread_mutex_unlock(&download_mutex);
    }
    return rc;
}

/**
 * Select first DOWNLOAD & INSTALL task from queue
 * @param [out]info information about task
 * @return 0 if task exists
 *         not 0 if there are not DOWNLOAD & INSTALL tasks in queue
 */
int scm_select_download_install_job(scm_job_description *info)
{
    int rc = 0;
    rc = scm_get_job_from_queue("DownloadInstall", info);
    if (0 == rc &&
        SCM_JOB_WAS_PUSH_TO_QUEUE == info->state &&
        0 == pthread_mutex_trylock(&downloadinstall_mutex)) {
        //TODO put some data below
        rc += scm_verify_conditions_ok(50, 100);
        if (0 == rc) { // verify OK
            scm_set_job_state_in_queue(SCM_JOB_IS_PROCESSING, info->job_key);
            rc += scm_download_package();
        }
        if(0 == rc) {
            rc += scm_install_package();
        }
        if (0 == rc) {// download & install OK
            scm_set_job_state_in_queue(SCM_JOB_IS_FINISHED_SUCCESS, info->job_key);
            scm_pop_job_from_queue(info->job_key);
        } else {
            scm_set_job_state_in_queue(SCM_JOB_IS_FAILED, info->job_key);
        }

        pthread_mutex_unlock(&downloadinstall_mutex);
    }
    return rc;
}

/**
 * Select first DOWNLOAD & INSTALL & INACTIVE task from queue
 * @param [out]info information about task
 * @return 0 if task exists
 *         not 0 if there are not DOWNLOAD & INSTALL & INACTIVE tasks in queue
 */
int scm_select_download_install_inactive_job(scm_job_description *info)
{
    int rc = 0;
    rc = scm_get_job_from_queue("DownloadInstallInactive", info);
    if (0 == rc &&
        SCM_JOB_WAS_PUSH_TO_QUEUE == info->state &&
        0 == pthread_mutex_trylock(&inactive_mutex)) {
        //TODO put some data below
        rc += scm_verify_conditions_ok(50, 100);
        if (0 == rc) { // verify OK
            scm_set_job_state_in_queue(SCM_JOB_IS_PROCESSING, info->job_key);
            rc += scm_download_package();
        }
        if(0 == rc) {
            rc += scm_install_package();
        }
        if(0 == rc) {
            rc += scm_do_inactive();
        }

        if (0 == rc) { // download & install & inactive OK
            scm_set_job_state_in_queue(SCM_JOB_IS_FINISHED_SUCCESS, info->job_key);
            scm_pop_job_from_queue(info->job_key);
        } else {
            scm_set_job_state_in_queue(SCM_JOB_IS_FAILED, info->job_key);
        }

        pthread_mutex_unlock(&inactive_mutex);
    }
    return rc;
}

/**
 * Routine
 * @param ptr
 */
static void* job_d_routine(void *ptr)
{
    scm_job_description *info = calloc(1, sizeof(scm_job_description));
    scm_select_download_job(info);
    //TODO there you can use "info" if you need
    free(info);
    return NULL;
}

static void* job_di_routine(void *ptr)
{
    scm_job_description *info = calloc(1, sizeof(scm_job_description));
    scm_select_download_install_job(info);
    //TODO there you can use "info" if you need
    free(info);
    return NULL;
}

static void* job_diia_routine(void *ptr)
{
    scm_job_description *info = calloc(1, sizeof(scm_job_description));
    scm_select_download_install_inactive_job(info);
    //TODO there you can use "info" if you need
    free(info);
    return NULL;
}

/**
 * Main data extraction cycle
 * @return 0 of OK
 *         1 if error
 */
int scm_main_loop()
{
    pthread_t download_thread;
    pthread_t install_thread;
    pthread_t inactive_thread;

    mutex_init_all();
    int mock_counter = 5;
    // We can execute only one operation of a same type a time
    while(0 < mock_counter){
        int rc = 0;
        rc += pthread_create(&download_thread, NULL, job_d_routine, NULL);
        rc += pthread_create(&install_thread, NULL, job_di_routine, NULL);
        rc += pthread_create(&inactive_thread, NULL, job_diia_routine, NULL);
        if(rc != 0)
            return 1;
        mock_counter--;
    }
    pthread_join(download_thread, NULL);
    pthread_join(install_thread, NULL);
    pthread_join(inactive_thread, NULL);

    mutex_destroy_all();
    return 0;
}
