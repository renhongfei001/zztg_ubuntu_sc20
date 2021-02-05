/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

/**
 * OMADM Interface Handler Module implementation
 */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omadm_interface_handler.h"
#include "pal.h"

#include "dm_logger.h"

#define MAX_FIELD_SIZE 512
#define REQUEST_TIMEOUT 8000 //extend the time out


static pal_http_options_t * post_request_options = NULL;
static data_buffer_t send_buffer;
static data_buffer_t receive_buffer;

static void* palHandle = NULL;
static long (*getPalSendHttpPostRequest)(char *, void *, void *, void *);


static void * alloc(long size)
{
    void *n = malloc(size);
    if (n)
        memset(n, 0, size);
    return n;
}

static void clone_data(char **dst, const char *src)
{
    free(*dst);
    if (src)
        *dst = strdup(src);
    else
        *dst = NULL;
}

/**
 * generate content type field for request
 * @param[in] format_type determine data content will be wbxml or not
 */
static void create_content_type_field(bool format_type)
{
    clone_data(&post_request_options->hdr_content_data, format_type?
            "Content-Type: application/vnd.syncml.dm+wbxml":
            "Content-Type: application/vnd.syncml.dm+xml");
}

/**
 * generate authentication field for request
 * @param[in] auth_data  SyncML authentication data
 */
static void create_auth_field(char *auth_data)
{
    char str[MAX_FIELD_SIZE] = "";

    switch(post_request_options->auth_type)
    {
        case AUTH_BASIC:
            /* Example:
             * Authorization: Basic NTI2OTJhMDAwNjYxODkwY
             */
            strcpy(str, "Authorization: Basic ");
            if(auth_data)
                strcat(str, auth_data);
            break;

        case AUTH_DIGEST:
            /* Example:
             * Authorization: Digest username="123",
             * nonce="nonce_value",
             * realm="realm",
             * response="md5_value"
             */
            strcpy(str,"Authorization: Digest ");
            if(post_request_options->username)
            {
                strcat(str, "username=\"");
                strcat(str, post_request_options->username);
                strcat(str, "\", ");
            }
            if(auth_data)
            {
                strcat(str, "response=\"");
                strcat(str, auth_data);
                strcat(str, "\"");
            }
            break;

        case AUTH_HMAC:
            /* Example:
             * x-syncml-hmac:algorithm=MD5, username="user",
             * mac=NTI2OTJhMDAwNjYxODkwYmQ3NWUxN2RhN2ZmYmJlMzk)
             */
            strcpy(str,"x-syncml-hmac: algorithm=MD5, ");
            if(post_request_options->username)
            {
                strcat(str, "username=\"");
                strcat(str, post_request_options->username);
                strcat(str, "\", ");
            }
            if(auth_data)
            {
                strcat(str, "mac=");
                strcat(str, auth_data);
            }
            break;

        case AUTH_X509:
            /// \todo: Certificate check - for now it’s unclear how to perform this
            break;

        default:
            /* do nothing */
            break;
    }
    clone_data(&post_request_options->hdr_auth_data, str);
}

/**
 * function for remapping dmclt_authType_t type to pal_auth_type_t
 * @param[in] auth_type dmclt_authType_t data
 * @return pal_auth_type_t data
 */
static pal_auth_type_t dmclt_to_pal_auth_type(dmclt_authType_t auth_type) {
    pal_auth_type_t ret;

    switch(auth_type)
    {
        case DMCLT_AUTH_TYPE_UNKNOWN:
            ret = AUTH_UNKNOWN;
            break;
        case DMCLT_AUTH_TYPE_HTTP_BASIC:
            ret = AUTH_BASIC;
            break;
        case DMCLT_AUTH_TYPE_HTTP_DIGEST:
            ret = AUTH_DIGEST;
            break;
        case DMCLT_AUTH_TYPE_HMAC:
            ret = AUTH_HMAC;
            break;
        case DMCLT_AUTH_TYPE_X509:
            ret = AUTH_X509;
            break;
        default:
            ret = AUTH_UNKNOWN;
            break;
    }

    return ret;
}

static int setup_connection() {
    DM_LOGI("IH: setup_connection() ");
    int status = DMCLT_ERR_NONE;

    palHandle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    if (palHandle == NULL) {
        DM_LOGE("IH: ERR palHandle == NULL ");
        return DMCLT_ERR_INTERNAL;
    }
    getPalSendHttpPostRequest = dlsym(palHandle, "pal_send_http_post_request");
    if (getPalSendHttpPostRequest == NULL)
    {
        dlclose(palHandle);
        DM_LOGE("IH: ERR getPalSendHttpPostRequest symbol not found");
        return DMCLT_ERR_INTERNAL;
    }

    post_request_options = alloc(sizeof(*post_request_options));
    if (post_request_options == NULL)
    {
        DM_LOGE("IH: ERR !!!! ALLOC !!!! ");
        return DMCLT_ERR_MEMORY;
    }

    post_request_options->hdr_content_data = alloc(MAX_FIELD_SIZE);
    if (post_request_options->hdr_content_data == NULL)
        goto FAIL_HDR_CONTENT_DATA;

    post_request_options->hdr_auth_data = alloc(MAX_FIELD_SIZE);
    if (post_request_options->hdr_auth_data == NULL)
        goto FAIL_HDR_AUTH_DATA;

    memset(&send_buffer, 0, sizeof(data_buffer_t));
    memset(&receive_buffer, 0, sizeof(data_buffer_t));

    DM_LOGI("IH: setup_connection() returning - %d", status);
    return status;

    FAIL_HDR_AUTH_DATA:
        DM_LOGI("IH: FAIL_HDR_AUTH_DATA");
        free(post_request_options->hdr_content_data);
        post_request_options->hdr_content_data = NULL;
    FAIL_HDR_CONTENT_DATA:
        DM_LOGI("IH: FAIL_HDR_CONTENT_DATA");
        free(post_request_options);
        post_request_options = NULL;
        DM_LOGE("IH: ERR returning 3 == DMCLT_ERR_MEMORY");
        return DMCLT_ERR_MEMORY;
}

dmclt_err_t omadm_interface_handler_post_package(dmclt_buffer_t* packet,
        dmclt_buffer_t* reply, bool format_type,
        char *username, char *password) {

    DM_LOGI("IH: post_package() called for username = %s | password = %s", username, password);

    int status = DMCLT_ERR_NONE;
    int http_err = 200;

    if (!packet || !reply)
    {
        DM_LOGI("IH: post_package() [1] returning status = %d", status);
        return DMCLT_ERR_INTERNAL;
    }

    if (!post_request_options)
    {
        status = setup_connection();
        if (status != DMCLT_ERR_NONE)
        {
            DM_LOGI("IH: post_package() [2] returning status = %d", status);
            return status;
        }
    }

    create_content_type_field(format_type);
    create_auth_field(packet->auth_data);

    post_request_options->auth_type = dmclt_to_pal_auth_type(packet->auth_type);

    clone_data(&post_request_options->username, username);
    // temporary disabling - since it seems to be passed as NULL for now
//    if (post_request_options->username == NULL) {
//        DM_LOGE("IH: post_package() [3]");
//        return DMCLT_ERR_MEMORY;
//    }
    clone_data(&post_request_options->password, password);
    // temporary disabling - since it seems to be passed as NULL for now
//    if (post_request_options->password == NULL) {
//        DM_LOGE("IH: post_package() [4]");
//        return DMCLT_ERR_MEMORY;
//    }

    /// \todo: it will be added later when we will be testing TLS functionality
    clone_data(&post_request_options->ca_fname, NULL);
    clone_data(&post_request_options->client_cert, NULL);
    clone_data(&post_request_options->client_key, NULL);

    post_request_options->timeout = REQUEST_TIMEOUT;

    send_buffer.data = packet->data;
    send_buffer.size = packet->length;

    http_err = getPalSendHttpPostRequest(packet->uri, post_request_options,
            &send_buffer, &receive_buffer);
    DM_LOGI("IH: pal_send_http_post_request() returning http_err = %d", http_err);
    if (200 != http_err) {
        status = DMCLT_ERR_ABORT;
    }

    if (receive_buffer.size != 0)
    {
        DM_LOGI("IH: post_package() GOT data size = %d ", receive_buffer.size);
        reply->data = receive_buffer.data;
        reply->length = receive_buffer.size;
    } else {
        reply->data = NULL;
        reply->length = 0;
    }
    memset(&send_buffer, 0, sizeof(data_buffer_t));
    memset(&receive_buffer, 0, sizeof(data_buffer_t));

    DM_LOGI("IH: post_package() returning status = %d", status);
    return status;
}

void omadm_interface_handler_end_session() {
    if (palHandle)
    {
        dlclose(palHandle);
        palHandle = NULL;
    }

    if (post_request_options)
    {
        if (post_request_options->hdr_content_data)
        {
            free(post_request_options->hdr_content_data);
            post_request_options->hdr_content_data = NULL;
        }
        if (post_request_options->hdr_auth_data)
        {
            free(post_request_options->hdr_auth_data);
            post_request_options->hdr_auth_data = NULL;
        }
        if (post_request_options->username)
        {
            free(post_request_options->username);
            post_request_options->username = NULL;
        }
        if (post_request_options->password)
        {
            free(post_request_options->password);
            post_request_options->password = NULL;
        }
        if (post_request_options->ca_fname)
        {
            free(post_request_options->ca_fname);
            post_request_options->ca_fname = NULL;
        }
        if (post_request_options->client_cert)
        {
            free(post_request_options->client_cert);
            post_request_options->client_cert = NULL;
        }
        if (post_request_options->client_key)
        {
            free(post_request_options->client_key);
            post_request_options->client_key = NULL;
        }

        free(post_request_options);
        post_request_options = NULL;
    }
}
