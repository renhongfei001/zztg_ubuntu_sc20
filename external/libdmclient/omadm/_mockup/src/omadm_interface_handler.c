/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omadm_interface_handler.h"
#include "pal.h"

#define BUFFER_MAX_SIZE 16384

static int count_package = 0;

// Reading sample server replay from file
int get_out_data(data_buffer_t * buffer, char *filename)
{
    int error = 0;
    FILE * fd;
    fd = fopen(filename, "r");
    if (!fd) {
        printf("OMADM IH: Can not open file %s\r\n", filename);
        return 1;
    }
    buffer->data = (char *) malloc(BUFFER_MAX_SIZE);
    memset(buffer->data, 0, BUFFER_MAX_SIZE * sizeof(char));
    buffer->size = fread(buffer->data, 1, BUFFER_MAX_SIZE, fd);
    if (buffer->size <= 0) {
        printf("OMADM IH: Can not read file %s\r\n", filename);
        error = 1;
    } else {
        printf("OMADM IH: Read  %d byte\r\n", (int) buffer->size);
    }
    fclose(fd);
    return error;
}

dmclt_err_t omadm_interface_handler_post_package(dmclt_buffer_t* packet,
        dmclt_buffer_t* reply, bool format_type, char *username, char *password)
{
    int err = 0, status = DMCLT_ERR_INTERNAL;
    data_buffer_t buffer;

    printf("MOCKUP omadm_interface_handler_post_package\n");
    count_package = count_package + 1;
    switch (count_package) {
    case 1: // first server reply packet forming
        err = get_out_data(&buffer,
                OMADM_MOCKUP_DIR"/test_server_replay_1.xml");
        break;
    case 2: // second server reply packet forming
        err = get_out_data(&buffer,
                OMADM_MOCKUP_DIR"/test_server_replay_2.xml");
        break;
    case 3: // third server reply packet forming
        err = get_out_data(&buffer,
                OMADM_MOCKUP_DIR"/test_server_replay_3.xml");
        count_package = 0;
        break;
    default:
        err = 1;
        count_package = 0;
        break;
    }
    if (err == 0 && NULL != buffer.data) {
        reply->data = buffer.data;
        reply->length = buffer.size;
        status = DMCLT_ERR_NONE;
    }

    return status;
}

void omadm_interface_handler_end_session()
{
    printf("MOCKUP omadm_interface_handler_end_session\n");
    count_package = 0;
}
