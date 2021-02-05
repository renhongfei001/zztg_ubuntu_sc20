/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef MO_OMADMTREE_H_
#define MO_OMADMTREE_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
    @brief node's type
*/
typedef enum
{
    OMADM_NODE_NOT_EXIST,
    OMADM_NODE_IS_INTERIOR,
    OMADM_NODE_IS_LEAF
} omadmtree_node_kind_t;

/**
    @brief node's description for plagin's users
*/
typedef struct
{
    char *uri;
    char *format;
    char *type;
    unsigned int data_size;
    char *data_buffer;
} dmtree_node_t;

/*!
 * @brief Callback to initialize the MO (MANDATORY)
 *
 * @param [out] dataP opaque pointer to MO internal data
 *
 * @returns a MO error code
 */
typedef int (*omadm_mo_init_fn) (void ** dataP);

/*!
 * @brief Callback to free the MO
 *
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 */
typedef void (*omadm_mo_close_fn) (void * data);

/*!
 * @brief Callback to get the type of a node
 *
 * @param [in] uri URL of the node
 * @param [out] type type of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
typedef int (*omadm_mo_is_node_fn) (const char * uri,
                                omadmtree_node_kind_t * type, void * data);

/*!
 * @brief Callback to find the URLs associated to an URN
 *
 * @param [in] urn URN to find
 * @param [out] urlsP null-terminated array of urls, freed by the caller
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
typedef int (*omadm_mo_find_urn_fn) (const char * urn, char *** urlsP,
                                     void * data);

/*!
 * @brief Callback to set the value of a node
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
typedef int (*omadm_mo_get_fn) (dmtree_node_t * nodeP, void * data);

/*!
 * @brief Callback to get the value of a node
 *
 * The targeted node can already exist. This is used both for ADD
 * and REPLACE commands.
 * If nodeP-> type is "node", an interior node must be created.
 *
 * @param [in] nodeP the node to store
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
typedef int (*omadm_mo_set_fn) (const dmtree_node_t * nodeP, void * data);

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
typedef int (*omadm_mo_get_ACL_fn) (const char * uri, char ** aclP,
                                    void * data);

/*!
 * @brief Callback to set the ACL of a node
 *
 * @param [in] uri URL of the node
 * @param [in] acl ACL of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
typedef int (*omadm_mo_set_ACL_fn) (const char * uri, const char *acl,
                                    void * data);

/*!
 * @brief Callback to rename a node
 *
 * The to parameter contains only the new name of the node, not the
 * complete new URL.
 *
 * @param [in] from URL of the node
 * @param [in] to new name of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
typedef int (*omadm_mo_rename_fn) ( const char * from, const char * to,
                                    void * data);

/*!
 * @brief Callback to delete a node
 *
 * @param [in] uri URL of the node
 * @param [in] data MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a MO error code
 */
typedef int (*omadm_mo_delete_fn) (const char * uri, void * data);

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
typedef int (*omadm_mo_exec_fn) (const char * uri, const char * cmdData,
                                 const char * correlator, void * data);


/*!
 * @brief Structure containing the interface of the MO
 *
 * base_uri and initFunc must be set. Other callbacks can be null.
 * The MO can not be root (i.e. base_uri must differ from ".").
 *
 */
typedef struct
{
    char * base_uri;                    /*!< base URI of the MO */
    omadm_mo_init_fn      initFunc;     /*!< initialization function */
    omadm_mo_close_fn     closeFunc;
    omadm_mo_is_node_fn   isNodeFunc;
    omadm_mo_find_urn_fn  findURNFunc;
    omadm_mo_get_fn       getFunc;
    omadm_mo_set_fn       setFunc;
    omadm_mo_get_ACL_fn   getACLFunc;
    omadm_mo_set_ACL_fn   setACLFunc;
    omadm_mo_rename_fn    renameFunc;
    omadm_mo_delete_fn    deleteFunc;
    omadm_mo_exec_fn      execFunc;
} omadm_mo_interface_t;


/*!
 * @brief Entry point of the shared lib
 *
 * The returned pointer ust be allocated by this function.
 * The caller will call closeFunc (if any) before freeing the pointer.
 * The caller will also free the uri string inside.
 *
 * @returns a pointer tothe MO interface
 */
omadm_mo_interface_t * omadm_get_mo_interface(void);


/**
    @brief Descriptor of event for DM-client
*/
typedef struct dmclt_item_t
{
    char * source; /*!< in, node name */
    char * target; /*!< in, vendor's server name */
    char * type; /*!< in, urn\url */
    char * format; /*!< in, MIME-type */
    char * data; /*!< in, event description */
} dmclt_item_t;

/**
    @brief Description of event passed from MO to DM-client
*/
typedef struct omadm_mo_event_t
{
    char *correlator; /*!< in, correlator was passed by Exec command */
    dmclt_item_t dmclt_item; /*!< in, Descriptor of event for DM-client */
} omadm_mo_event_t;

/**
    @brief Callback for sending event into DM-client
    @param [in] event, event descriptor
    @return 0 if sucess, errno if fail
 */
typedef int (*omadm_mo_ft_event_handler) (omadm_mo_event_t *event);

/**
    @brief Registering event handler into MO plugin.
    @param [in] data, data was got from initFunc result
    @param [in] event_handler, callback, pointer to event handler
        in DM-client
    @return 0 if sucess, errno if fail
*/
int omadm_mo_register_event_handler(void *data,
                                omadm_mo_ft_event_handler event_handler);

/**
    @brief Unregistering event handler into MO plugin.
    @param [in] data, data was got from initFunc result
    @param [in] event_handler, callback, pointer to event handler
        in DM-client
    @return 0 if sucess, errno if fail
*/
int omadm_mo_unregister_event_handler(void *data,
                                omadm_mo_ft_event_handler event_handler);


#ifdef __cplusplus
}
#endif

#endif
