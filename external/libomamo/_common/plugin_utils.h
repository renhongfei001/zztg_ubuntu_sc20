/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef PLUGIN_UTILS_H_
#define PLUGIN_UTILS_H_

#include "mo_omadmtree.h"
#include "pal.h"

#define OMADM_NODE_FORMAT        "node"
#define OMADM_LEAF_FORMAT_BOOL   "bool"
#define OMADM_LEAF_FORMAT_CHAR   "chr"
#define OMADM_LEAF_FORMAT_INT    "int"
#define OMADM_LEAF_FORMAT_FLOAT  "float"
#define OMADM_LEAF_TYPE          "text/plain"
#define OMADM_NODE_TYPE          "node"
#define OMADM_NOT_EXIST          "none"

typedef char* (*omadm_mo_get_leaf_fn) ();
typedef int (*omadm_mo_set_leaf_fn) (void * data);

/**
* Struct for plugin tree node
*/
typedef struct
{
    char * uri;                /// URI
    char * urn;                /// URN
    char * format;             /// Format
    char * type;               /// Type
    char * acl;                /// ACL
    char * value;              /// Value
    char * getLeafFuncName;    /// Function's name for getting value
    char * setLeafFuncName;    /// Function's name for setting value
    char * execLeafFuncName;   /// Function's name for execution
} plugin_tree_node_t;

/**
* Enum with codes for plugin state
*/

enum plugin_states {
    PLUGIN_NODE_NOT_EXIST = -1,    /// Plugin node is not exist
    PLUGIN_SUCCESS = 0,            /// Plugin function complete successfully
    PLUGIN_ERROR = 1,              /// Plugin function  not complete successfully
};

///It is internal struct definition that we use
///to create internal dynamic list of node's childs
typedef struct child{
    char * node;
    struct child *next;
} child_t;

/**
 *  Find node in array
 *  @param[in] node Array with all plugin nodes
 *  @param[in] iURI Internal node
 *  @return first node with selected uri
 */
int prv_find_node(plugin_tree_node_t * node, const char * iURI);

/**
 *  Find uri with urn
 *  @param[in] urn of plugin
 *  @param[out] uri of node
 *  @param[in] node Array with all plugin nodes
 *  @return error code
 */
int prv_find_urn(const char *iURN, char ***oURL, void *iData);

/**
 *  Get acl string
 *  @param[in] uri of node
 *  @param[out] strig with acl value
 *  @param[in] node Array with all plugin nodes
 *  @return error code
 */
int prv_get_acl_fn(const char *iURI, char **oValue, void *iData);

/**
 *  Set acl string
 *  @param[in] uri of node
 *  @param[in] strig with acl value
 *  @param[in] node Array with all plugin nodes
 *  @return error code
 */
int prv_set_acl_fn(const char *iURI, const char *acl, void * iData);

/**
 *  Rename node
 *  @param[in] uri of node
 *  @param[out] new uri of node
 *  @param[in] node Array with all plugin nodes
 *  @return error code
 */
int prv_rename_fn (const char *from, const char *to, void *iData);

/**
 *  Delete node from tree
 *  @param[in] uri of node
 *  @param[in] node Array with all plugin nodes
 *  @return error code
 */
int prv_delete_fn (const char *iURI, void *iData);

/**
 *  Check node type
 *  @param[in] uri of node
 *  @param[out] type of node
 *  @param[in] node Array with all plugin nodes
 *  @return error code
 */
int prv_mo_is_node(const char *iURI, omadmtree_node_kind_t *oNodeType, void *iData);

/**
 *  Gets all children of the interior node
 *  @param[out] node Array with all plugin nodes
 *  @param[out] iURI Internal node
 *  @return String with child's names separated by '/'
 */
char* get_child_list (plugin_tree_node_t *node, const char * iURI);

/**
 *  Adds childrens of the interior node to the list
 *  @param[out] p_begin begin of the list
 *  @param[out] child new element
 *  @return Begin of the list
 */
child_t* add_child_to_list (child_t* p_begin, char* child);

/**
 *  Converts list with children to string
 *  @param[out] p_begin begin of the list
 *  @param[out] childs_in_string string buffer
 *  @return String with child's names separated by '/'
 */
char * list_to_string (child_t* p_begin, char * childs_in_string);

/**
 *  Gets values from .txt file
 *  @param[out] gNodes Array with all plugin nodes
 *  @param[out] way Way for .txt file
 *  @return Error code
 */
int read_from_file(plugin_tree_node_t *gNodes, char * way);

/**
 *  Copy values from default init txt file to current
 *  @param[out] cur_way Way for current txt file
 *  @param[out] way Way for default txt file
 *  @return Error code
 */
int copy_init_to_current (char * way, char * cur_way);

/**
 *  Write values to current  txt file
 *
 *  Used when prv_set_fn is called
 *  @param[out] nodes Array with all plugin nodes
 *  @param[out] cur_way Way for current txt file
 *  @return Error code
 */
int set_to_file(plugin_tree_node_t * nodes, char * cur_way);

/**
 *  Checks path for Android.
 *
 *  If folder "data" doesn't exist
 *  function will create "data" folder.
 *  @param[out] cur_way Way for current txt file
 *  @return Error code
 */
void check_path(char * cur_way);

/**
 *  Parses X parameter for DIAGMON and SCM nodes.
 *
 *  @param[out] uri String with requested uri
 *  @param[out] x Parameter for URI
 *  @return Error code
 */
int parsing_x(char *, char **);

/**
 * Frees buffer
 *
 * @param[out] buffer Contains data_buffer_t data
 */
void free_buffer(data_buffer_t *);

void *getPalHandle(char *pal_path);

int releasePalHandle(void *palHandle);
#endif /* PLUGIN_UTILS_H_ */
