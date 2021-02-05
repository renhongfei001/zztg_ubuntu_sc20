/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef SCM_H
#define SCM_H

#include <dlfcn.h>
#include <linux/limits.h>
#include "dm_logger.h"
#include "mo_omadmtree.h"
#include "mo_error.h"
#include "pal.h"
#include "scm_error.h"

/**
    @brief SCM_BASE_URI, base uri for SCOMO
*/
#define SCM_BASE_URI "./SCM"

/**
    @brief SCM_URN, SCOMO Management Object identifier
*/
#define SCM_URN "urn:oma:mo:oma-scomo:1.0"

/**
 * @brief SCM_NODE_ABILITY_NONE, Node's posibility to do a nothing
 */
#define SCM_NODE_ABILITY_NONE                            ((unsigned int)0x0000)
/**
 * @brief SCM_NODE_ABILITY_EXEC, Node's posibility to do a command "Exec"
 */
#define SCM_NODE_ABILITY_EXEC                            ((unsigned int)0x0001)
/**
 * @brief SCM_NODE_ABILITY_GET, Node's posibility to do a command "Get"
 */
#define SCM_NODE_ABILITY_GET                             ((unsigned int)0x0002)
/**
 * @brief SCM_NODE_ABILITY_REPLACE, Node's posibility to do a command "Replace"
 */
#define SCM_NODE_ABILITY_REPLACE                         ((unsigned int)0x0004)

/**
 * @brief Status of the Download and/or Install operation:
 *        Download/<X>/Status
 */

/** @brief There is no data available and download is about to start */
#define SCM_STATE_IDLE                                   ((int)10)

/** @brief Download failed and there is No data received */
#define SCM_DOWNLOAD_STATE_DOWNLOAD_FAILED               ((int)20)

/** @brief Denotes that a download has started and that 0 or
 *         more bytes of data have been downloaded
 */
#define SCM_DOWNLOAD_STATE_DOWNLOAD_PROGRESSING          ((int)30)

/** @brief Have data after Download has been completed successfully */
#define SCM_DOWNLOAD_STATE_DOWNLOAD_COMPLETED            ((int)40)

/** @brief Denotes that an installation has started */
#define SCM_DOWNLOAD_STATE_INSTALL_PROGRESSING           ((int)50)

/** @brief Install failed and the downloaded package is still in the Device */
#define SCM_DOWNLOAD_STATE_INSTALL_FAILED_WITH_DATA      ((int)60)

/** @brief Install failed and the downloaded package is deleted */
#define SCM_DOWNLOAD_STATE_INSTALL_FAILED_WITHOUT_DATA   ((int)70)


/**
 * @brief status of Remove or Install Primitive
 *        Inventory/Delivered/<X>status
 */
/** @brief Remove failed and the Deployment Component is still in the Device*/
#define SCM_DELIVERED_STATE_REMOVE_FAILED                ((int)20)

/** @brief Denotes that Remove has started */
#define SCM_DELIVERED_STATE_REMOVE_PROGRESSING           ((int)30)

/** @brief Denotes that an installation has started */
#define SCM_DELIVERED_STATE_INSTALL_PROGRESSING          ((int)40)

/** @brief Install failed and the downloaded package is still in the Device */
#define SCM_DELIVERED_STATE_INSTALL_FAILED_WITH_DATA     ((int)50)

/** @brief Install failed and the downloaded package is deleted */
#define SCM_DELIVERED_STATE_INSTALL_FAILED_WITHOUTH_DATA ((int)70)

/**
 * @brief Status of RemovePrimitive.
 *        Inventory/Deployed/<X>/Status
 */
/** @brief Remove failed and the component is still in the Device */
#define SCM_DEPLOYED_STATE_REMOVE_FAILED                 ((int)20)

/** @brief Denotes that Remove has started */
#define SCM_DEPLOYED_STATE_REMOVE_PROGRESSING            ((int)30)

/** @brief Activate failed and the component is still in the Inactive state */
#define SCM_DEPLOYED_STATE_ACTIVATE_FAILED               ((int)40)

/** @brief Denotes that the Activate operation has started */
#define SCM_DEPLOYED_STATE_ACTIVATE_PROGRESSING          ((int)50)

/** @brief Deactivate failed and the component is still in the Active state */
#define SCM_DEPLOYED_STATE_DEACTIVATE_FAILED             ((int)60)

/** @brief Denotes that the Deactivate operation has started */
#define SCM_DEPLOYED_STATE_DEACTIVATE_PROGRESSING        ((int)70)

/**
  * @brief  The Delivery Package states:
  */
/** @brief The Delivery Package is in the Delivered State */
#define SCM_DELIVERY_PACKAGE_STATE_DELIVERED             ((int)10)

/** @brief The Delivery Package is in the Installed State */
#define SCM_DELIVERY_PACKAGE_STATE_INSTALLED             ((int)20)

/**
 * @brief  The Deployment Component states:
 */
/** @brief  The Deployment Component is in the Inactive State */
#define SCM_DEPLOYMENT_COMPONENT_STATE_INACTIVE          ((int)10)

/** @brief  The Deployment Component is in the Active State */
#define SCM_DEPLOYMENT_COMPONENT_STATE_ACTIVE            ((int)20)

/**
  * @brief SCOMO job states:
  */
#define SCM_JOB_UNDEFINED_STATE                          ((int)0)
#define SCM_JOB_WAS_PUSH_TO_QUEUE                        ((int)1)
#define SCM_JOB_IS_PROCESSING                            ((int)2)
#define SCM_JOB_IS_FAILED                                ((int)3)
#define SCM_JOB_IS_FINISHED_SUCCESS                      ((int)4)
#define SCM_JOB_IS_CANCELLED                             ((int)5)
#define SCM_JOB_IS_DEFERRED                              ((int)6)
#define SCM_JOB_IS_PREPARED_FOR_DELETE                   ((int)7)
#define SCM_JOB_IS_DELETED                               ((int)8)

/*!
 * @brief Callback to initialize the MO
 * @param [out] dataP opaque pointer to MO internal data
 * @returns a MO error code
 */
int scm_root_init_fn(void **dataP);

/*!
 * @brief Callback to free the MO
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 */
void scm_root_close_fn(void *data);

/*!
 * @brief Callback to get the type of a node
 * @param [in] uri URL of the node
 * @param [out] type type of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_is_node_fn(const char *uri, omadmtree_node_kind_t *type,
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
int scm_root_find_urn_fn (const char * urn, char *** urlsP, void * data);

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
int scm_root_get_fn (dmtree_node_t * nodeP, void * data);

/*!
 * @brief Callback to set the value of a node
 *
 * The targeted node can already exist. This is used both for ADD
 * and REPLACE MO commands.
 * If nodeP-> type is "node", an interior node must be created.
 * @param [in] nodeP the node to store
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 * @returns a MO error code
 */
int scm_root_set_fn (const dmtree_node_t * nodeP, void * data);

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
int scm_root_get_acl_fn (const char * uri, char ** aclP, void * data);

/*!
 * @brief Callback to set the ACL of a node
 *
 * @param [in] uri URL of the node
 * @param [in] acl ACL of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
int scm_root_set_acl_fn(const char * uri, const char *acl, void * data);

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
int scm_root_exec_fn (const char * uri, const char * cmdData,
                             const char * correlator, void * data);

/**
 * @breif Job’s description
 */
typedef struct scm_job_description {
    int pkg_primary_key;       /*!< primary key of <x> package */
    char *package_name;        /*!< Package name */
    char *package_parent_node; /*!< Parent node: download/delivered/deployed */
    char *cmd_data;            /*!< Input argument for node’s execution */
    char *correlator;          /*!< Data for sending alert to DM-Server */
    int state;                 /*!< State of job */
    int job_key;               /*!< key of job, primary key in queue table */

} scm_job_description;

/**
    @brief g_dmclient_event_handler callback for DM-client event handler
*/
omadm_mo_ft_event_handler g_dmclient_event_handler;

/**
* @brief It pushes job to queue in Data Base. Job is command EXEC on node
*        is defined by uri.
* @param [in] uri, uri’s node
* @param [in] info, structure with information whith contains patameters of job
* @return zero if operation was successful
**/
int push_job_to_queue(const char *pkg_name, const char *parent_uri, const char* command,
                        const char *cmd_data, const char *correlator);
/*
int scm_push_job_to_queue(const char *uri, const scm_job_description *info);
*/

/**
* @brief It gets job from queue in Data Base. Job is command EXEC on node is
*        defined by uri. Important: job isn’t deleted from queue
* @param [in] node command type
* @param [out] scm job parameters
* @return zero if operation was successful
**/
int get_job_from_queue(const char* executed_node, scm_job_description** job_description);

/**
 * @breif It pops (removes) job from queue in Data Base. Job is command EXEC on node is defined by uri.
 * @param [in] job_key, key of job, primary key in queue table
 * @return zero if operation was successful
 */
int pop_job_from_queue(int job_key);

/**
 * @breif It sets job state at queue in Data Base. Job is command EXEC on node is defined by uri.
 * @param [in] job_key, key of job, primary key in queue table
 * @param [in] state, State of job
 * @return zero if operation was successful
**/
int set_job_state_in_queue(int job_key, int state);

/**
    @brief node format in enum
*/
typedef enum scm_node_format {
    efnf_Unknown = 0,
    efnf_Node,
    efnf_Char,
    efnf_Bin,
    efnf_Int,
    efnf_Bool
} scm_node_format;

/**
    @brief convering scm_node_format to string of type
    @param [in] nf, node's format as type
    @param [out] format, node's format, shall be freeded by "free"
    @return !0 if memory isn't enough
*/
int get_string_node_type(scm_node_format nf, char **type);

/**
    @brief convering scm_node_format to string of format
    @param [in] nf, node's format as type
    @param [out] format, node's format, shall be freeded by "free"
    @return !0 if memory isn't enough
*/
int get_string_node_format(scm_node_format nf, char **format);

/** @brief converting string of node's format to scm_node_format
 *         and omadmtree_node_kind_t
 *  @param [in ]format, node's format as string
 *  @param [out] nf, node's format as type
 *  @param [out] nk, node's kind as type
 *  @return !0 if input string is null
 */
int get_node_type_from_string(scm_node_format *nf, omadmtree_node_kind_t *nk,
                              char *format);

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
    @brief struct scm_node_property, node's property
*/
typedef enum
{
   NODE_URI    = 0,    /*!< URI */
   NODE_URN    = 1,    /*!< URN */
   NODE_ACL    = 2,    /*!< node's ACL */
   NODE_FORMAT = 3,    /*!< node's format */
   NODE_VALUE  = 4,    /*!< node's value  */
   NODE_KIND   = 5,    /*!< node's kind */
   NODE_ABILITY= 6,    /*!< node's posibility to do a commands */
} scm_node_property;

/**
    @brief node's value
*/
typedef struct scm_node_value {
    char* data_;        /*!< data as chars array, must be "free()" */
    unsigned int size_; /*!< size of data in bytes */
} scm_node_value;

/**
    @brief struct scm_node, node's description
*/
typedef struct scm_node {
    char *uri_;                         /*!< URI */
    char *urn_;                         /*!< URN */
    char *acl_;                         /*!< node's ACL */
    omadmtree_node_kind_t node_kind_;   /*!< node type */
    scm_node_format format_;            /*!< node format */
    scm_node_value value_;              /*!< node's value */
    unsigned int node_command_ability_; /*!< node's posibility to do a
                                             commands */
} scm_node;

/**
 * @brief DB API
 */
/**
 * @brief It read node from db table
 * @param [in] uri, node’s uri
 * @param [out] node, pointer to the node structure
 * @return zero if operation was successful
 **/
int scm_get_node(const char * uri, scm_node **node);

/**
 * @brief It write node to the db table
 * @param [in] uri, node’s uri
 * @param [in] node, pointer to the node structure
 * @return zero if operation was successful
 **/
int scm_set_node(const char * uri, scm_node * node);

/**
 * @brief It write specified node's field in table
 * @param [in] uri, node’s uri
 * @param [in] prop, it specify the required field
 * @param [in] data, data for update
 * @return zero if operation was successful
 **/
int scm_set_node_prop(const char * uri, scm_node_property prop, char *data);

/**
 * @brief It read specified node's field from the table
 * @param [in] uri, node’s uri
 * @param [in] prop, it specify the required field
 * @param [out] data, the required data
 * @return zero if operation was successful
 **/
int scm_get_node_prop(const char * uri, scm_node_property prop, char **data);

/**
 * @brief It init sqlite lib
 * @return zero if operation was successful
 **/
int init_lib(void);

/**
 * @brief It init database
 * @return zero if operation was successful
 **/
int init_database(void);

/**
 * @brief It close database
 * @return zero if operation was successful
 **/
int close_database(void);


/** @brief removes all temporary files and sets state to default */
void scm_cleanup(void);

/**
 *  @brief Check pre-condition for Exec command
 *  @param [in] uri, node’s uri
 *  @param [out] pname, package name
 *  @param [out] parent_uri, node's parent uri
 *  @param [out] command, the Exec command
 *  @return zero if operation was successful
 */
int check_exec_pre_condition(const char * uri, char **pname, char **parent_uri, char **command);

/**
 *  @brief Gets all leafs for interior node
 *  @param [out] id node’s id from data base
 *  @param [out] node scm node
 *  @return zero if operation was successful
 */
int get_childs(char * id, scm_node ** node);

#define MIN_REQ_BATTERY 50
#define MIN_REQ_MEMORY 100

int scm_download_package();
int scm_install_package();
int scm_do_inactive();

#endif // SCM_H
