/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scm.h"
#include "scm_error.h"
#include "mo_omadmtree.h"

#define MOCK_URI "./SCM/download/pck1"
#define NODES_COUNT 3
#define SCM_PKG1_DATA "Package1 data"
#define SCM_PKG2_DATA "Package2 data"

typedef struct scm_mock_nodes {
    scm_node nodes_[NODES_COUNT];  /*!< array of static nodes */
} scm_mock_nodes;

scm_mock_nodes g_mock_nodes = {
    { // nodes
      {
          SCM_BASE_URI "/Download/Package1", NULL, "Get=*&Replace=*",
          OMADM_NODE_IS_LEAF, efnf_Char,
          { SCM_PKG1_DATA, sizeof(SCM_PKG1_DATA)-1 },
          SCM_NODE_ABILITY_GET|SCM_NODE_ABILITY_REPLACE|SCM_NODE_ABILITY_EXEC
      },
      {
          SCM_BASE_URI "/Download/Package2" , NULL,"Get=*&Replace=*",
          OMADM_NODE_IS_LEAF, efnf_Char,
          { SCM_PKG2_DATA, sizeof(SCM_PKG2_DATA)-1 },
          SCM_NODE_ABILITY_EXEC
      },
    }
};


int scm_get_job_from_queue(const char *node, scm_job_description *info)
{
    if (NULL == info)
        return 1;
    info->pkg_primary_key = 0;
    info->state = SCM_JOB_WAS_PUSH_TO_QUEUE;
    info->job_key = 0;
    info->cmd_data = strdup("cmd_data");
    info->correlator = strdup("correlator");
    return 0;
}

int scm_pop_job_from_queue(const int job_key)
{
    if (0 > job_key)
        return 1;
    else
        return 0;
}

int scm_set_job_state_in_queue(int state, int job_key)
{
    if (0 > state || 0 > job_key)
        return 1;
    else
        return 0;
}
