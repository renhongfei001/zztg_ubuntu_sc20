/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>

#include "mo_error.h"
#include "pal.h"
#include "plugin_utils.h"
#include "dm_logger.h"

/**
 * Finds the node's index with plugin_tree_node_t array
 *
 * @param[in] node Array with all plugin nodes
 * @param[in] iURI Internal node
 * @return Index of the node.
 *         \code PLUGIN_NODE_NOT_EXIST \endcode If the required node is not found
 */
int prv_find_node(plugin_tree_node_t * nodes,
                         const char * iURI)
{
    int i = PLUGIN_NODE_NOT_EXIST;
    if (nodes != NULL && iURI != NULL){
        i = 0;
        while(strcmp(nodes[i].uri,"NULL"))
        {
            if(!strcmp(iURI, nodes[i].uri)) return i;
            i++;
        }
        if (!strcmp(nodes[i].uri,"NULL"))
        {
            return PLUGIN_NODE_NOT_EXIST;
        }
    }
    return i;
}

/** Find URN function
 *
 * @param[in] iURI Node URI
 * @param[in] oURL
 * @param[in] iData Data with plugin tree nodes
 * @return \code PLUGIN_SUCCESS
 *         PLUGIN_ERROR \endcode
 */
int prv_find_urn(const char *iURN,
                      char ***oURL,
                      void *iData)
{
    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;
    int i;
    int count;

    if (!nodes || !oURL || !iURN)
        return PLUGIN_ERROR;

    *oURL = NULL;
    count = 0;
    i = 0;
    while(strcmp(nodes[i].uri,"NULL"))
    {
        if (nodes[i].urn && !strcmp(iURN, nodes[i].urn))
        {
            char ** tmpP;
            count++;
            tmpP = (char **)malloc((count + 1) * sizeof(char*));
            if (NULL == tmpP)
            {
                if (NULL != *oURL)
                {
                    int j;
                    j = 0;
                    while(NULL != (*oURL)[j])
                    {
                        free((*oURL)[j]);
                        *oURL[j] = NULL;
                        j++;
                    }
                    free(*oURL);
                    *oURL = NULL;
                }
                return PLUGIN_ERROR;
            }
            tmpP[count - 1] = strdup(nodes[i].uri);
            if(tmpP[count - 1] == NULL){
                free(tmpP);
                tmpP = NULL;
                return MO_ERROR_DEVICE_FULL;
            }
            tmpP[count] = NULL;
            if (count > 1)
            {
                memcpy(tmpP, *oURL, (count-1) * sizeof(char*));
                free(*oURL);
                *oURL = NULL;
            }
            *oURL = tmpP;
        }
        i++;
    }

    if (count > 0)
    {
        return PLUGIN_SUCCESS;
    }
    return PLUGIN_ERROR;
}

/**
* Get ACL function
*
* @param[in] iURI Node URI
* @param[out] oValue
* @param[in] iData Data with plugin tree nodes
* @return \code PLUGIN_SUCCESS
*         PLUGIN_ERROR \endcode
*/
int prv_get_acl_fn(const char *iURI,
                     char **oValue,
                     void *iData)
{
    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;
    int i;
    int err = PLUGIN_SUCCESS;
    if (!nodes || !oValue || !iURI)
        return PLUGIN_ERROR;

    *oValue = NULL;

    i = prv_find_node(nodes, iURI);
    if (i == -1) return MO_ERROR_NOT_FOUND;

    if (strcmp(nodes[i].type ,OMADM_NOT_EXIST))
    {
        if (nodes[i].acl)
        {
            *oValue = strdup(nodes[i].acl);
            if (!*oValue) err = PLUGIN_ERROR;
        }
    }

    return err;
}

/**
*  Check node type
*  @param[in] uri of node
*  @param[out] type of node
*  @param[in] iData Data with plugin tree nodes
*  @return Count of entries of '/' character in a string
*/

int prv_mo_is_node(const char *iURI, omadmtree_node_kind_t *oNodeType, void *iData)
{
    plugin_tree_node_t * nodes = (plugin_tree_node_t *)iData;
    int i;

    if (!nodes)
        return MO_ERROR_COMMAND_FAILED;

    i = prv_find_node(nodes, iURI);
    if (i != PLUGIN_NODE_NOT_EXIST){
        if (strcmp(nodes[i].type, OMADM_NODE_TYPE) == 0){
            *oNodeType = OMADM_NODE_IS_INTERIOR;
        } else if(strcmp(nodes[i].type, OMADM_LEAF_TYPE) == 0){
            *oNodeType = OMADM_NODE_IS_LEAF;
        }
    } else {
        *oNodeType = OMADM_NODE_NOT_EXIST;
    }
    return MO_ERROR_NONE;
}

/**
*  Сounts the number of entries of a character in a string
*  @param[in] string Text for counting of '/'
*  @param[in] symbol Symbol which you need to find in the string
*  @return Count of entries of '/' character in a string
*/

int find_slash_count(const char *string, char symbol)
{
    int count = 0;
    while(*string != '\0')
    {
        if(*string == symbol) ++count;
        ++string;
    }
    return count;
}

/**
*  Adds a new element with name of a child at the end of a dynamic list
*  @param[out] p_begin Head pointer of a list
*  @param[in] child Name of a child node which you want to add
*  @return Head pointer of a list with the new element
*/
child_t* add_child_to_list (child_t* p_begin, char* child)
{
    child_t* p_current;
    p_current = p_begin;

    while (p_current->next != NULL)
    {
        p_current = p_current->next;
    }

    child_t* new_child = (child_t *)malloc(sizeof(child_t));
    if (new_child != NULL){
        new_child->node = child;
        new_child->next = NULL;
        p_current->next = new_child;
    }
    return p_begin;
}

/**
*  Adds a new element with name of a child at the end of a dynamic list
*  @param[out] p_begin Head pointer of a list
*  @param[out] childs_in_string String for the node's names
*  @return String with names separated by '/'
*/
char * list_to_string (child_t* p_begin, char * childs_in_string)
{
    child_t* p_current;
    int size = 0;

    // looping through the list to calculate the size of the childs_in_string
    p_current = p_begin;
    while(p_current->next)
    {
        size += strlen(p_current->node) + 1;
        p_current = p_current->next;
    }
    size += strlen(p_current->node) + 1;

    childs_in_string = (char*)malloc(sizeof(char) * (size + 1));
    if(childs_in_string != NULL){
        memset(childs_in_string, 0, sizeof(char) * (size + 1));
        // looping through the list to add the node's name
        //to the end of the childs_in_string
        p_current = p_begin;
        while(p_current->next){
            strncat(childs_in_string,p_current->node,size);
            p_current = p_current->next;
        }
        strncat(childs_in_string,p_current->node,size);
    }
    return childs_in_string;
}

char* get_child_list (plugin_tree_node_t *gNodes, const char *iURI)
{
    int i = 0;
    int iURI_count = 0;
    child_t* p_begin = NULL;

    iURI_count = find_slash_count(iURI, '/'); //Calculate '/' in iURI
    int first = 1;

    while (strcmp(gNodes[i].uri,"NULL"))
    {
        int URI_count = 0; //Number of'/' in URI node

        //In the case where the iURI is contained in gNodes[i].uri
        //add child's name to the string
        if (strstr(gNodes[i].uri,iURI)) //In case
            {
                URI_count = find_slash_count(gNodes[i].uri, '/');

                //add the child's name only if the difference between
                //numbers of '/' is 1
                if ( (URI_count - iURI_count) == 1 )
                {
                    char *ach;
                    ach = strrchr (gNodes[i].uri,'/'); //determine the position of the last '/'
					printf("abing get_child_list = %s\n", ach);
                    if (!first) // for the not first element of the dynamic list
                    {
                        p_begin = add_child_to_list(p_begin,ach);
                    }
                    else
                    {
                        p_begin = (child_t *)malloc(sizeof(child_t));
                        p_begin->node = ach+1;
                        p_begin->next = NULL;
                        first = 0;
                    }
                }
            }
        i++;
    }
    char * string = NULL;
    if (p_begin != NULL) //if node is not empty
    {
        // convert from the list to the string
        string = list_to_string(p_begin, string);
        child_t* p_current = NULL;
        child_t* p_prev = NULL;
        for (p_current = p_begin; p_current; )
        {
           p_prev = p_current;
           p_current  = p_current->next;
           free(p_prev);
           p_prev = NULL;
         }
         p_begin = NULL;
         return string;
         }
    else return NULL;
}

int read_from_file(plugin_tree_node_t * gNodes, char * way)
{
    char * read;
    char line[255];
    line[0]='\0';
    size_t length = 255;
    char * first = NULL;
    char * second = NULL;

    FILE *file;
    if ((file = fopen(way,"r")) == NULL){
        DM_LOGE("read_from_file(): Could not open file for reaed: %s", way);
        return PLUGIN_ERROR;
    }
    rewind(file);

    while (1){
        read = fgets(line, length, file);
        if (read == NULL){
            if ( feof (file) != 0){
                break;
            } else {
                return PLUGIN_ERROR;
            }
        }

        first = strtok(line,"|");
        second = strtok (NULL, "\n");
        int node_index = 0;
        char* text = NULL;
        text = strdup((const char *)first);
        if (!text) {
            return PLUGIN_ERROR;
        }

        node_index = prv_find_node(gNodes, (const char*)text);
        free(text);
        text = NULL;
        if ( (node_index != -1) && (second != NULL) ){
            gNodes[node_index].value = strdup((const char *)second);
            if (!gNodes[node_index].value) return PLUGIN_ERROR;
        }
}
    fclose(file);
    return PLUGIN_SUCCESS;
}

int copy_init_to_current(char * way, char * cur_way)
{
    FILE *file;
    FILE *cur_file;
    char * read;
    char line[255];
    line[0]='\0';
    size_t length = 255;

    if ((file = fopen(way,"r")) == NULL){
        DM_LOGE("copy_init_to_current(): Could not open file for reaed: %s", way);
        return PLUGIN_ERROR;
    }
    rewind(file);

    if ((cur_file = fopen(cur_way,"w")) == NULL){
        DM_LOGE("copy_init_to_current(): Could not open file for write: %s", cur_way);
        if (file) fclose(file);
        return PLUGIN_ERROR;
    }

    while (1){
        read = fgets(line, length, file);
        if (read == NULL){
            if ( feof (file) != 0){
                break;
            } else {
                if (file) fclose(file);
                if (cur_file) fclose(cur_file);
                return PLUGIN_ERROR;
            }
        }

        fputs (line, cur_file);
    }

    if (file) fclose(file);
    if (cur_file) fclose(cur_file);
    return PLUGIN_SUCCESS;
}

int set_to_file(plugin_tree_node_t * nodes, char * cur_way)
{
    FILE * cur_file;
    int i = 0;
    if ((cur_file = fopen(cur_way,"w")) == NULL){
        DM_LOGE("set_to_file(): Could not open file for write: %s", cur_way);
        return PLUGIN_ERROR;
    }
    rewind(cur_file);

    while (strcmp(nodes[i].uri,"NULL")){
        if ( !strcmp(nodes[i].format,OMADM_LEAF_FORMAT_CHAR) &&
             strcmp(nodes[i].value,"")){ // if LEAF
            char * insert = NULL;
            int input_len = (strlen(nodes[i].uri)+1+strlen(nodes[i].value)+1);
            insert = (char*)malloc(sizeof(char)*(input_len+1));
            strcpy(insert,"");
            //insert uri
            strncat(insert,(const char*)nodes[i].uri,strlen(nodes[i].uri));
            strncat(insert,"|",1);
            //insert value
            strncat(insert,(const char*)nodes[i].value,strlen(nodes[i].value));
            strncat(insert,"\n",1);
            fputs (insert, cur_file);
            free(insert);
            insert = NULL;
        }
        i++;
    }

    fclose(cur_file);
    return PLUGIN_SUCCESS;
}

static void _mkdir(const char *dir)
{
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for(p = tmp + 1; *p; p++){
        DM_LOGI("*** MKDIR dir_part = %s", tmp);
        if(*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(tmp, S_IRWXU);
}

void check_path(char * cur_way)
{
    struct stat st = {0};
    if ((stat(cur_way, &st) == -1)) {
        DM_LOGI("*** MKDIR dir = %s", cur_way);
        _mkdir(cur_way);
    }
}

int parsing_x(char *uri, char **x){

    char *uri_copy;
    char *x_param = NULL;
    int error = MO_ERROR_NONE;

    DM_LOGI("<X>: start parsing <X> for uri = %s", uri);
    uri_copy = strdup(uri);
    if (uri_copy){
        x_param = strtok(uri_copy, "/");
        int i = 0;
        for(i = 0; i<4; i++){
            x_param = strtok(NULL, "/");
        }
        if (x_param){
            *x = strdup (x_param);
            if (*x == NULL) error = MO_ERROR_DEVICE_FULL;
        } else error = MO_ERROR_INCOMPLETE_COMMAND;
    } else error = MO_ERROR_DEVICE_FULL;
    if (uri_copy){
        free(uri_copy);
    }
    return error;
}

void free_buffer(data_buffer_t *buffer)
{
    if (buffer){
        if (buffer->data){
            free(buffer->data);
            buffer->data = NULL;
        }
      free(buffer);
      buffer = NULL;
    }
}

void *getPalHandle(char *pal_path)
{
	int (* qmi_open_handle)();
	void *palHandle = NULL;
    palHandle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);

    if (!palHandle){
        DM_LOGE( "palHandle not initialised %s", dlerror());
        return NULL;
    }

    qmi_open_handle = dlsym(palHandle, "idev_QMI_open");
    if (qmi_open_handle)
    {
        qmi_open_handle();
    } 

	return palHandle;
}

int releasePalHandle(void *palHandle)
{
	int res = -1;
	void (* qmi_release_handle)();
    if (palHandle) {
        // Not sure if we can do anything with error code provided by dlclose(),
        // so just print error and exit.
        res = dlclose(palHandle);
        palHandle = NULL;
        if (0 != res)
           DM_LOGE("%s: palHandle not closed %s", __FILE__, dlerror());

        qmi_release_handle = dlsym(palHandle, "idev_QMI_release");

        if (qmi_release_handle)
        {
            qmi_release_handle();
        }
    }

	return res;
}
