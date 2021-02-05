/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */


#include <dirent.h>
#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#include "dm_logger.h"
#include "mo_error.h"
#include "mo_omadmtree.h"
#include "plugin_utils.h"
#include "scm.h"

/*HOWTO:
 * create simple app:
 * void main()
 *  {
 * init_lib();
 * init_data_base();
 * init_insert();
 * }
 *
 * data base will created and some row
 */

static void* sqlite_lib = NULL;

int (*sqlite_open)(const char*,sqlite3**,int, const char* );
int (*sqlite_request)(sqlite3*, const char*,int (*callback)(void*,int,char**,char**), void *, char ** );
int (*sqlite_close)(sqlite3*);

int (*sqlite_prepare_v2)(sqlite3 *, const char *, int, sqlite3_stmt **, const char **);
int (*sqlite_step)(sqlite3_stmt*);
const void *(*sqlite_column_text)(sqlite3_stmt*, int);
int (*sqlite_finalize)(sqlite3_stmt *);


sqlite3 *db = NULL;
scm_node *result_node = NULL;
char *node_data = NULL;

int  init_jobs_database(void);

void clean_data(char** string){
    if(*string){
        free(*string);
        *string = NULL;
    }
}

/**
 * Open sqlite lib
 * @return sqlite lib open state
 *         \code MO_ERROR_COMMAND_FAILED \endcode If lib not found/ failed to open
 *         \code MO_ERROR_NONE \endcode if lib open succesfully
 */

int init_lib(void)
{

    sqlite_lib = dlopen(SQLITE_LIB_DIR"/"SQLITE_LIB_NAME, RTLD_LAZY);

    if (!sqlite_lib) {
        DM_LOGI( "sqlite_lib not initialised %s", dlerror());
        return MO_ERROR_COMMAND_FAILED;
    }

    sqlite_open = dlsym(sqlite_lib, "sqlite3_open_v2");
    sqlite_request = dlsym(sqlite_lib, "sqlite3_exec");
    sqlite_close = dlsym(sqlite_lib, "sqlite3_close");
    sqlite_prepare_v2 = dlsym(sqlite_lib, "sqlite3_prepare_v2");
    sqlite_step = dlsym(sqlite_lib, "sqlite3_step");
    sqlite_column_text = dlsym(sqlite_lib, "sqlite3_column_text");
    sqlite_finalize = dlsym(sqlite_lib, "sqlite3_finalize");

    if (!sqlite_open || !sqlite_request || !sqlite_close ||
            !sqlite_prepare_v2 || !sqlite_step || !sqlite_column_text
            || !sqlite_finalize){
        DM_LOGI( "sqlite_lib not initialised %s", dlerror());
        return MO_ERROR_COMMAND_FAILED;
    }
    return MO_ERROR_NONE;
}

int open_database(bool create){

    if(sqlite_open(DATA_BASE_LOCALION"/"DATA_BASE_NAME, &db,  create? SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE : SQLITE_OPEN_READWRITE, NULL))
    {
        DM_LOGE("Database connection failed\n");
        sqlite_close(db);
        db = NULL;
        return -1;
    }

    DM_LOGI("Connection successful\n");
    return MO_ERROR_NONE;
}
/**
 * Open and first init for scomo database
 * @return error for database init
 *  0 if data base open/create succesfully
 */

int init_database(void)
{
    int err = 0;
    err = init_lib();
    if(err == 0 && open_database(true) == MO_ERROR_NONE){

        char *create_table = "CREATE TABLE IF NOT EXISTS scomo (id integer , parent integer, node_name text,"
                         " acl text, type integer, format integer, value text, ability integer,"
                         " primary key (id autoincrement), foreign key (parent) references scomo(id)"
                         " on update cascade on delete cascade);";

        // Execute the query for creating the table

        err = sqlite_request(db ,create_table,0,0,0);
        DM_LOGI("create status %d\n",err );

        char *insert_root_nodes = "insert or replace into scomo values (1, null, 'SCM', 'Get=*&Replace=*',1,1,null,7);"
                            "insert or replace into scomo values (2, 1, 'download' , 'Get=*&Replace=*&Add=*',1,1,null,7);"
                            "insert or replace into scomo values (3, 1, 'inventory' , 'Get=*&Replace=*&Add=*',1,1,null,7);"
                            "insert or replace into scomo values (4, 3, 'delivered' , 'Get=*&Replace=*&Add=*',1,1,null,7);"
                            "insert or replace into scomo values (5, 3, 'deployed' , 'Get=*&Replace=*&Add=*',1,1,null,7);"
                            "insert or replace into scomo values (6, 2, 'Package1' , 'Get=*&Replace=*',2,2,null,7);";

        err = sqlite_request(db,insert_root_nodes,0,0,0);
        DM_LOGI("insert status %d\n",err );
        init_jobs_database();
        sqlite_close(db);
        db = NULL;
    }
    return err;
}

int init_jobs_database(void)
{
    int err = 0;
    DM_LOGI("SCOMO DB: init_jobs_database");
    err = init_lib();
    if(err == 0 && open_database(true) == MO_ERROR_NONE){

        char *create_table = "CREATE TABLE IF NOT EXISTS jobs (id integer , pkg_primary_key integer, package_name text,"
                         " package_parent_node text, executed_node text, cmd_data text, correlator text, state integer,"
                         " primary key (id autoincrement), foreign key (pkg_primary_key) references jobs(id)"
                         " on update cascade on delete cascade);";

        // Execute the query for creating the table

        err = sqlite_request(db ,create_table,0,0,0);

        sqlite_close(db);
        db = NULL;
    }
    return err;
}

/**
 * Close sqlite lib
 * @return sqlite lib close error
 */
int close_database(void){
    if(result_node){
        clean_data(&result_node->acl_);
        clean_data(&result_node->uri_);
        clean_data(&result_node->value_.data_);
        free(result_node);
        result_node = NULL;
    }
    if(node_data){
        free(node_data);
        node_data = NULL;
    }
    if (sqlite_lib) {
        // Not sure if we can do anything with error code provided by dlclose(),
        // so just print error and exit.
        int res = dlclose(sqlite_lib);
        sqlite_lib = NULL;
        if (0 != res) {
           DM_LOGE("sqlite_lib not closed %s", dlerror());
            return -1;
        }
    }
    return MO_ERROR_NONE;
}

int select_callback(void *pArg, int argc, char **argv, char **columnNames){

    pArg = 0;
    int i;
    for(i = 0; i < argc; i++) {

        DM_LOGE(" %s = %s\n", columnNames[i], argv[i] ? argv[i] : "NULL");
    }

    if(argc == 1){
        if(argv[0]) {
            node_data = strdup(argv[0]);
        } else {
            node_data = strdup("NULL");
        }
    } else {
        node_data =  strdup(argv[0]);
        result_node = calloc(1,sizeof(scm_node));
        result_node->acl_= strdup(argv[2] ? argv[2] : "NULL");
        DM_LOGE("%s\n", argv[2] ? argv[2] : "NULL");
        result_node->format_ = (scm_node_format)atoi(argv[4]);
        DM_LOGE("%d\n", result_node->format_);
        result_node->node_kind_= (omadmtree_node_kind_t)atoi(argv[3]);
        DM_LOGE("%d\n", result_node->node_kind_);
        result_node->value_.data_ = strdup(argv[5] ? argv[5] : "NULL");
        result_node->value_.size_ = strlen(result_node->value_.data_);
        DM_LOGE("%s\n", result_node->value_.data_);
        result_node->node_command_ability_ = (unsigned int)atoi(argv[6]);
        DM_LOGE("%u\n", result_node->node_command_ability_);
        DM_LOGE("\n");
    }
    return MO_ERROR_NONE;
}

int job_select_callback(void *result_job, int argc, char **argv, char **columnNames){

    scm_job_description **jd = (scm_job_description**)result_job;
    *jd = (scm_job_description*)calloc(1,sizeof(scm_job_description));
    int i;
    for(i = 0; i < argc; i++) {
        DM_LOGI(" %s = %s\n", columnNames[i], argv[i] ? argv[i] : "NULL");
    }

    (*jd)->pkg_primary_key = (int)atoi(argv[1]);
    DM_LOGI("%d\n", (*jd)->pkg_primary_key);
    (*jd)->package_name = strdup(argv[2] ? argv[2] : "NULL");
    DM_LOGE("%s\n", argv[2] ? argv[2] : "NULL");
    (*jd)->package_parent_node = strdup(argv[3] ? argv[3] : "NULL");
    DM_LOGE("%s\n", argv[3] ? argv[3] : "NULL");
    (*jd)->cmd_data = strdup(argv[4] ? argv[4] : "NULL");
    DM_LOGE("%s\n", argv[4] ? argv[4] : "NULL");
    (*jd)->correlator = strdup(argv[5] ? argv[5] : "NULL");
    DM_LOGE("%s\n", argv[5] ? argv[5] : "NULL");
    (*jd)->state = (int)atoi(argv[6]);
    DM_LOGI("%d\n", (*jd)->state);
    (*jd)->job_key = (int)atoi(argv[0]);
    DM_LOGI("%d\n", (*jd)->job_key);
    DM_LOGI("\n");

    return MO_ERROR_NONE;
}

/**
 * Get node from database
 * @return get error
 */
int scm_get_node(const char *uri,scm_node **node){
    char *err_msg;
    int err = 0;
    *node  = NULL;
    if(open_database(false) != MO_ERROR_NONE){
        return MO_ERROR_COMMAND_FAILED;
    }


    char* select = "with recursive cte as ( "
                   "select "
                   "id, "
                   "parent, "
                   "acl, "
                   "type, "
                   "format, "
                   "value, "
                   "ability, "
                   "'./' || node_name as uri "
                   "from scomo where parent is null "
                   "union all "
                   "select "
                   "scomo.id, "
                   "scomo.parent, "
                   "scomo.acl, "
                   "scomo.type, "
                   "scomo.format, "
                   "scomo.value, "
                   "scomo.ability, "
                   "cte.uri || '/' || scomo.node_name "
                   "from cte, scomo on scomo.parent = cte.id "
                   ") "
                   "select * from cte where uri='%s';";
    DM_LOGI("node content : %s",select);
    char get_node_request[strlen(select)+strlen(uri)];
    snprintf(get_node_request, sizeof(get_node_request),select, uri);
    DM_LOGI("node content : %s",get_node_request);

    if(result_node){
        clean_data(&result_node->acl_);
        clean_data(&result_node->uri_);
        clean_data(&result_node->value_.data_);
        free(result_node);
        result_node = NULL;
    }

    DM_LOGI("SCOMO DB: scm_get_node result_node = %p",result_node);
    clean_data(&node_data);
    err = sqlite_request(db,get_node_request,select_callback,0,&err_msg);

    DM_LOGI("request state : %s",err_msg);
    if (err == 0 && result_node){
        result_node->uri_= strdup(uri);
        result_node->urn_= NULL;
        *node =  result_node;
        if (result_node->node_kind_ == OMADM_NODE_IS_INTERIOR){
            err = get_childs(node_data, &result_node);
        } else err = MO_ERROR_NONE;
    }

    if(!result_node) {
        err = MO_ERROR_NOT_FOUND;
    }
    clean_data(&node_data);
    sqlite_close(db);
    db = NULL;
    return err;
}

/**
 * Set node to database
 * @return set error
 */
int scm_set_node(const char * uri, scm_node * node){
    char *err_msg;
    int err = 0;

    DM_LOGI("SCOMO DB: scm_set_node uri = %s",uri);

    if(open_database(false) != MO_ERROR_NONE){
        return MO_ERROR_COMMAND_FAILED;
    }
    DM_LOGI("SCOMO_DB: scm set node %s\n", uri);
    char* select = "with recursive cte as ( "
                   "select "
                   "id, "
                   "parent, "
                   "acl, "
                   "type, "
                   "format, "
                   "value, "
                   "ability, "
                   "'./' || node_name as uri "
                   "from scomo where parent is null "
                   "union all "
                   "select "
                   "scomo.id, "
                   "scomo.parent, "
                   "scomo.acl, "
                   "scomo.type, "
                   "scomo.format, "
                   "scomo.value, "
                   "scomo.ability, "
                   "cte.uri || '/' || scomo.node_name "
                   "from cte, scomo on scomo.parent = cte.id "
                   ") "
                   "select id from cte where uri='%s';";
    char get_node_request[strlen(select)+strlen(uri)];
    snprintf(get_node_request, sizeof(get_node_request), select, uri);
    node_data = NULL;
    err = sqlite_request(db,get_node_request,select_callback,0,&err_msg);
    if (node_data){
        char * full_update_request = "update scomo set acl = '%s' where id = %s ; "
                                 "update scomo set type = %d where id = %s ; "
                                 "update scomo set format = %d where id = %s ; "
                                 "update scomo set value = '%s' where id = %s ; "
                                 "update scomo set ability = %d where id = %s ; ";
        char * full_set_request = NULL;
        int size = strlen(full_update_request) + strlen(node->acl_)+2+strlen(node->value_.data_)+1+5*strlen(node_data)+10;
        full_set_request = (char*)malloc(sizeof(char) * size);

        snprintf(full_set_request, size,
                full_update_request,
                node->acl_ , node_data,
                node->node_kind_,node_data,
                node->format_ ,node_data,
                node->value_.data_,node_data,
                node->node_command_ability_,node_data);
        err = sqlite_request(db,full_set_request,0,0,&err_msg);
        clean_data(&full_set_request);
    } else {
       char *child_node = NULL;
       char * uri_copy = strdup(uri);
       char * check_node_select = "select id from scomo where node_name = '%s' ;";
       char * child_node_check = "and parent = %d ;";
       char* add_node = "insert into scomo(parent,node_name, acl, type, format, value, ability) values (%d, '%s', '%s',%d,%d,'%s',%d);";
       char * node_request = NULL;
       int curr_id;
       int size;
       //create full select for child
       char * check_child_node_select = calloc(sizeof(char)*(strlen(check_node_select)-1+strlen(child_node_check)),1);
       strncpy(check_child_node_select,check_node_select,strlen(check_node_select)-1);
       strncat(check_child_node_select, child_node_check, strlen(child_node_check));

       if (uri_copy){
            child_node = strtok(uri_copy, "/");
            child_node = strtok(NULL, "/");
            clean_data(&node_request);
            size = strlen(check_node_select)+strlen(child_node);
            node_request = (char*)calloc(sizeof(char)*size,1);
            snprintf(node_request, size,check_node_select,child_node);
            node_data = NULL;
            err = sqlite_request(db,node_request,select_callback,0,&err_msg);
            if(node_data){
                curr_id = atoi(node_data);
                clean_data(&node_data);
                child_node = strtok(NULL, "/");
            } else {
                DM_LOGI("SCOMO DB: root node not found");
                clean_data(&uri_copy);
                clean_data(&check_child_node_select);
                return MO_ERROR_COMMAND_FAILED;
            }

            while (child_node != NULL) {
                clean_data(&node_request);
                size = strlen(check_child_node_select)+strlen(child_node)+ 4;
                node_request = (char*)calloc(sizeof(char)*size,1);
                snprintf(node_request, size,check_child_node_select,child_node,curr_id);
                node_data = NULL;
                err = sqlite_request(db,node_request,select_callback,0,&err_msg);
                if(node_data){
                    curr_id = atoi(node_data);
                    clean_data(&node_data);
                } else {
                    clean_data(&node_request);
                    if(!node->value_.data_){
                        size = strlen(add_node)+strlen(child_node) + 1 + strlen(node->acl_)+2+1+1+10;
                    } else
                        size = strlen(add_node)+strlen(child_node) + 1 + strlen(node->acl_)+2+strlen(node->value_.data_)+1+1+10;
                    node_request = (char*)calloc(sizeof(char)*size,1);
                    snprintf(node_request, size ,add_node ,curr_id,child_node,
                             node->acl_ , node->node_kind_, node->format_ ,node->value_.data_? node->value_.data_:"", node->node_command_ability_);
                    err = sqlite_request(db,node_request,0,0,&err_msg);
                    clean_data(&node_request);
                }
                child_node = strtok(NULL, "/");
            }

        }
        clean_data(&uri_copy);
        clean_data(&check_child_node_select);
    }
    sqlite_close(db);
    db = NULL;
    return err;
}

/**
 * Set node from database
 * @return get error
 */
int scm_set_node_prop(const char * uri, scm_node_property prop, char *data){
    char *err_msg;
    int err = 0;
    if(!data) {
        return MO_ERROR_COMMAND_FAILED;
    }

    if(open_database(false) != MO_ERROR_NONE){
        return MO_ERROR_COMMAND_FAILED;
    }
    DM_LOGI("SCOMO_DB: scm get node %s, prop %d\n", uri, prop);
    char* select = "with recursive cte as ( "
                   "select "
                   "id, "
                   "parent, "
                   "acl, "
                   "type, "
                   "format, "
                   "value, "
                   "ability, "
                   "'./' || node_name as uri "
                   "from scomo where parent is null "
                   "union all "
                   "select "
                   "scomo.id, "
                   "scomo.parent, "
                   "scomo.acl, "
                   "scomo.type, "
                   "scomo.format, "
                   "scomo.value, "
                   "scomo.ability, "
                   "cte.uri || '/' || scomo.node_name "
                   "from cte, scomo on scomo.parent = cte.id "
                   ") "
                   "select id from cte where uri='%s';";
    char get_node_request[strlen(select)+strlen(uri)];
    snprintf(get_node_request,sizeof(get_node_request), select, uri);
    clean_data(&node_data);
    err= sqlite_request(db,get_node_request,select_callback,0,&err_msg);
    char * update_request = "update scomo set %s = '%s' where id = %s ;";
    char * update_prop_request = NULL;
    switch(prop){
        case NODE_URI:
        case NODE_URN:
            break;
        case NODE_ACL:
            update_prop_request = (char*)calloc(sizeof(char) *(strlen(update_request)+strlen("acl")+strlen(data)),1);
            snprintf(update_prop_request, strlen(update_request)+strlen("acl")+strlen(data), update_request, "acl", data, node_data);
            err = sqlite_request(db,update_prop_request,0,0,&err_msg);
            clean_data(&update_prop_request);
            break;
        case NODE_FORMAT:
            update_prop_request = (char*)calloc(sizeof(char) *(strlen(update_request)+strlen("format")+strlen(data)),1);
            snprintf(update_prop_request, strlen(update_request)+strlen("format")+strlen(data),update_request, "format", data, node_data);
            err = sqlite_request(db,update_prop_request,0,0,&err_msg);
            clean_data(&update_prop_request);
            break;
        case NODE_KIND:
            update_prop_request = (char*)calloc(sizeof(char) *(strlen(update_request)+strlen("type")+strlen(data)),1);
            snprintf(update_prop_request, strlen(update_request)+strlen("type")+strlen(data), update_request, "type", data, node_data);
            err = sqlite_request(db,update_prop_request,0,0,&err_msg);
            clean_data(&update_prop_request);
            break;
        case NODE_VALUE:
            update_prop_request = (char*)calloc(sizeof(char) *(strlen(update_request)+strlen("value")+strlen(data)),1);
            snprintf(update_prop_request, strlen(update_request)+strlen("value")+strlen(data), update_request, "value", data, node_data);
            err = sqlite_request(db,update_prop_request,0,0,&err_msg);
            clean_data(&update_prop_request);
            break;
        case NODE_ABILITY:
            update_prop_request = (char*)calloc(sizeof(char) *(strlen(update_request)+strlen("ability")+strlen(data)),1);
            snprintf(update_prop_request, strlen(update_request)+strlen("ability")+strlen(data),update_request, "ability", data, node_data);
            err = sqlite_request(db,update_prop_request,0,0,&err_msg);
            clean_data(&update_prop_request);
            break;
        default:
            err = MO_ERROR_COMMAND_FAILED;
            break;
    }
    sqlite_close(db);
    db = NULL;
    return err;
}

char * property_request(char* select, char *prop, const char * uri){
    char * _request = (char *)calloc(sizeof(char)*(strlen(select)+ strlen(uri) + strlen(prop)),1);
    snprintf(_request,strlen(select)+ strlen(uri) + strlen(prop), select, prop, uri);
    return _request;
}

/**
 * Get node from database
 * @return get error
 */
int scm_get_node_prop(const char * uri, scm_node_property prop, char **data){
    char *err_msg;
    int err = 0;
    if(open_database(false) != MO_ERROR_NONE){
        return MO_ERROR_COMMAND_FAILED;
    }

    char* select = "with recursive cte as ( "
                   "select "
                   "id, "
                   "parent, "
                   "acl, "
                   "type, "
                   "format, "
                   "value, "
                   "ability, "
                   "'./' || node_name as uri "
                   "from scomo where parent is null "
                   "union all "
                   "select "
                   "scomo.id, "
                   "scomo.parent, "
                   "scomo.acl, "
                   "scomo.type, "
                   "scomo.format, "
                   "scomo.value, "
                   "scomo.ability, "
                   "cte.uri || '/' || scomo.node_name "
                   "from cte, scomo on scomo.parent = cte.id "
                   ") "
                   "select %s from cte where uri='%s';";
    DM_LOGI("node content : %s",select);
    char *prop_select = NULL;
    *data = NULL;

    if(node_data){
        free(node_data);
        node_data = NULL;
    }

    switch(prop){
        case NODE_URI:
        case NODE_URN:
            break;
        case NODE_ACL:{
            prop_select = property_request(select, "acl",uri);
            err = sqlite_request(db,prop_select,select_callback,0,&err_msg);
            break;
        }
        case NODE_FORMAT:{
        prop_select = property_request(select,"format",uri);
            err = sqlite_request(db,prop_select,select_callback,0,&err_msg);
            break;
        }
        case NODE_KIND:{
            prop_select = property_request(select,"type",uri);
            err = sqlite_request(db,prop_select,select_callback,0,&err_msg);
            break;
        }
        case NODE_VALUE:{
            prop_select = property_request(select,"value",uri);
            err = sqlite_request(db,prop_select,select_callback,0,&err_msg);
            break;
        }
        case NODE_ABILITY:{
            prop_select = property_request(select,"ability",uri);
            err = sqlite_request(db,prop_select,select_callback,0,&err_msg);
            break;
        }
    default:
        err = MO_ERROR_NOT_FOUND;
        break;
    }
    if (err == 0 && node_data){
        *data =  strdup(node_data);
        err = MO_ERROR_NONE;
    }
    if(!node_data){
        err = MO_ERROR_NOT_FOUND;
    }

    if(prop_select){
        free(prop_select);
        prop_select = NULL;
    }

    DM_LOGI("request state : %s",err_msg);

    sqlite_close(db);
    db = NULL;
    return err;
}

int push_job_to_queue(const char *pkg_name, const char *parent_uri, const char* command, const char *cmd_data, const char *correlator){
    char *err_msg;
    int err = 0;
    char *pkg_primary_key = NULL;
    char * select_req = NULL;
    if(open_database(false) != MO_ERROR_NONE){
        return MO_ERROR_COMMAND_FAILED;
    }
    DM_LOGI("SCOMO DB: push_job_to_queue pkg_name = %s command = %s",
            pkg_name ? pkg_name : "null",
            command ? command : "null");
    if(pkg_name){
        char * select = "select id from scomo where node_name = '%s';";
        select_req = (char *)calloc(sizeof(char)*(strlen(select)+strlen(pkg_name)),1);
        snprintf(select_req,strlen(select) + strlen(pkg_name), select, pkg_name);
        clean_data(&node_data);
        err = sqlite_request(db, select_req, select_callback, 0, &err_msg);
        if(node_data){
            pkg_primary_key = strdup(node_data);
            free(node_data);
            node_data = NULL;
        } else {
           clean_data(&select_req);
           sqlite_close(db);
           db = NULL;
           return MO_ERROR_NOT_FOUND;
        }
        clean_data(&select_req);
    } else
        return MO_ERROR_COMMAND_FAILED;

    char * add_job = "insert into jobs (pkg_primary_key,package_name,package_parent_node, executed_node,cmd_data, correlator,state)"
                   "            values (%s, '%s', '%s','%s', '%s', '%s', %d);";
    int size = strlen(add_job) + strlen(pkg_name) +strlen(pkg_primary_key)+
            strlen(parent_uri)+strlen(command?command:"")+strlen(cmd_data?cmd_data:"")+strlen(correlator?correlator:"")+2;
    select_req = (char *)calloc(sizeof(char)*size,1);
    snprintf(select_req,size,add_job,pkg_primary_key,pkg_name,parent_uri,command?command:"",cmd_data?cmd_data:"",correlator?correlator:"",1);
    err = sqlite_request(db,select_req,0,0,&err_msg);

    clean_data(&select_req);

    DM_LOGI("request state : %s error = %d",err_msg, err);

    clean_data(&pkg_primary_key);
    sqlite_close(db);
    db = NULL;
    return err;
}

int get_job_from_queue(const char* executed_node, scm_job_description** job_description){

    char *err_msg;
    int err = 0;

    char * select_req = NULL;
    if(open_database(false) != MO_ERROR_NONE){
        return MO_ERROR_COMMAND_FAILED;
    }

        char * select = "select * from jobs where executed_node = '%s' limit 1";
        select_req = (char *)calloc(sizeof(char)*(strlen(select)+strlen(executed_node)),1);
        snprintf(select_req,strlen(select) + strlen(executed_node), select, executed_node);
        err = sqlite_request(db, select_req, job_select_callback, job_description, &err_msg);
        clean_data(&select_req);

        sqlite_close(db);
        db = NULL;

    return MO_ERROR_NONE;
}

int pop_job_from_queue(int job_key){
    char *err_msg;
    int err = 0;
    char * select_req = NULL;
    DM_LOGI("SCOMO db: pop_job_from_queue");
    if(open_database(false) != MO_ERROR_NONE){
        return MO_ERROR_COMMAND_FAILED;
    }

    char * select = "delete from jobs where id = %d;";
    int size = strlen(select) + 4;
    select_req = (char *)calloc(sizeof(char)*size,1);
    snprintf(select_req,size,select,job_key);
    err = sqlite_request(db,select_req,0,0,&err_msg);
    clean_data(&select_req);
    DM_LOGI("SCOMO_DB: err_msg = %s err = %d",err_msg, err);
    sqlite_close(db);
    db = NULL;

    return err;
}
int set_job_state_in_queue(int job_key, int state){
    char *err_msg;
    int err = 0;
    char * select_req = NULL;
    if(open_database(false) != MO_ERROR_NONE){
        return MO_ERROR_COMMAND_FAILED;
    }

    char * select = "update jobs set state = %d where id = %d;";
    int size = strlen(select) + 8;
    select_req = (char *)calloc(sizeof(char)*size,1);
    snprintf(select_req,size,select,state,job_key);
    err = sqlite_request(db,select_req,0,0,&err_msg);

    clean_data(&select_req);

    sqlite_close(db);
    db = NULL;

    return err;
}

int get_childs(char * id, scm_node ** node){
    child_t* p_begin = NULL;
    int first = 1;
    int err = MO_ERROR_NONE;

    char * get_childs_request = NULL;
    sqlite3_stmt *stmt;
    int rc = 0;

    DM_LOGI("SCOMO DB: get childs for = %s", id);

    if (!db){
        if(open_database(false) != MO_ERROR_NONE){
            return MO_ERROR_COMMAND_FAILED;
        }
    }

    char * select = "select * from scomo where parent = %s;";
    int size = strlen(select)+strlen(id);
    get_childs_request = (char *)calloc(sizeof(char)*size,1);
    snprintf(get_childs_request, size, select, id);

    if ( (rc = sqlite_prepare_v2(db, get_childs_request, -1, &stmt, NULL)) != SQLITE_OK){
        clean_data(&get_childs_request);
        return MO_ERROR_COMMAND_FAILED;
    }

    while ((rc = sqlite_step(stmt)) == SQLITE_ROW){
        char *child = strdup(sqlite_column_text(stmt, 2));
        if (!child){
            clean_data(&get_childs_request);
            return MO_ERROR_DEVICE_FULL;
        }
        if (!first){ // for the not first element of the dynamic list
            p_begin = add_child_to_list(p_begin,"/");
            p_begin = add_child_to_list(p_begin,child);
        } else {
            p_begin = (child_t *)calloc(sizeof(child_t),1);
            p_begin->node = child;
            p_begin->next = NULL;
            first = 0;
        }
        DM_LOGI("SCOMO DB:Child = %s", child);
    }

    char * string = NULL;
    if (p_begin != NULL) //if node is not empty
    {
        string = list_to_string(p_begin, string);
        child_t* p_current = NULL;
        child_t* p_prev = NULL;
        for (p_current = p_begin; p_current; ){
            p_prev = p_current;
            p_current  = p_current->next;
            free(p_prev);
            p_prev = NULL;
        }

        p_begin = NULL;
        (*node)->value_.data_ = strdup(string);
        if (!(*node)->value_.data_ ){
            clean_data(&get_childs_request);
            clean_data(&string);
            return MO_ERROR_DEVICE_FULL;
        }
        (*node)->value_.size_=strlen((*node)->value_.data_);
        clean_data(&string);
    }

    sqlite_finalize(stmt);
    clean_data(&get_childs_request);
    clean_data(&string);
    return err;
}
