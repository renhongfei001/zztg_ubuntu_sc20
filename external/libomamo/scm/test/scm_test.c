/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "dm_logger.h"
#include "mo_error.h"
#include "mo_omadmtree.h"
#include "plugin_utils.h"
#include "scm.h"
#include "scm_error.h"

omadm_mo_interface_t* g_interface = 0;

/**
 * Initializes test suite
 *
 * Function loads pal from the pal directory
 * and loads plugin for test
 * @return 0 if success, -1 otherwise
 */
int init_suite(void)
{
    g_interface = omadm_get_mo_interface();
    return (g_interface != 0 ? 0 : 1);
}

/** The suite cleanup function.
 *
 * @return 0 on success, non-zero otherwise.
 */
int clean_suite(void)
{
    g_interface->closeFunc(NULL);

    if(g_interface) {
        if(g_interface->base_uri) {
            free(g_interface->base_uri);
        }
        free(g_interface);
    }
    return 0;
}

void test_case_omadm_get_mo_interface(void)
{
    DM_LOGI("SCOMO TEST: get_mo_interface");
    omadm_mo_interface_t* interface = omadm_get_mo_interface();
    CU_ASSERT_PTR_NOT_NULL_FATAL(interface);
    if(interface) {
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->base_uri);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->initFunc);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->closeFunc);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->isNodeFunc);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->findURNFunc);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->getFunc);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->setFunc);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->getACLFunc);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->setACLFunc);
        CU_ASSERT_PTR_NOT_NULL_FATAL(interface->execFunc);
        if(interface->base_uri) {
            free(interface->base_uri);
        }
        free(interface);
    }
}

void test_case_init_func(void)
{
    void *data = NULL;
    DM_LOGI("SCOMO TEST: test_case_init_func");
    CU_ASSERT_EQUAL(init_lib(),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(init_database(),MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&data),
        MO_ERROR_NONE );
};

void test_case_close_func(void)
{
    void *data = NULL;
    DM_LOGI("SCOMO TEST: test_case_close_func");
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&data),
        MO_ERROR_NONE );

    g_interface->closeFunc(data);

    data = NULL;
    g_interface->closeFunc(data);
};

void test_case_is_node_func(void)
{
    DM_LOGI("SCOMO TEST: test_case_is_node_func");
    char uriBad[] = "./ManagedObjects/SCM";
    char uriGood[] = "./SCM/download/Package1";

    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;
    void *data = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&data),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uriBad, 0, data),
                     MO_ERROR_COMMAND_FAILED);
    CU_ASSERT_EQUAL( g_interface->isNodeFunc(0, &type, data),
                     MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->isNodeFunc("", &type, data),
                     MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uriBad, &type, data),
                     MO_ERROR_NONE);
    CU_ASSERT_EQUAL (type,OMADM_NODE_NOT_EXIST);

    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uriGood, &type, data),
                     MO_ERROR_NONE);
                     //MO_ERROR_NOT_EXECUTED);

    g_interface->closeFunc(data);
};

void test_case_find_urn_func(void)
{
    DM_LOGI("SCOMO TEST: test_case_find_urn_func");
    char urn[] = SCM_URN;
    char urnBad[] = "bad:urn";
    char **urlsP = 0, **urls = 0;
    void *context = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->findURNFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->findURNFunc(0, &urlsP, context),
                     MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->findURNFunc(urnBad, &urlsP, context),
                     MO_ERROR_NOT_FOUND);

    CU_ASSERT_EQUAL( g_interface->findURNFunc(urn, &urlsP, context),
                     MO_ERROR_NONE);

    urls = urlsP;
    while(*urls) {
        DM_LOGI("TEST: free %p",*urls);
        free(*urls);
        ++urls;
    }
    free(urlsP);

    g_interface->closeFunc(context);
};

void test_case_get_func(void)
{
    DM_LOGI("SCOMO TEST: test_case_find_urn_func");

    char uri_dl[] = "./SCM/download/Package1";
    char uri_bad[] = "./SCM/download/Bad";
    dmtree_node_t nodeP;
    void *context = NULL;
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;

    memset(&nodeP, 0, sizeof(nodeP));
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uri_dl, &type, context),
                     MO_ERROR_NONE);
                      //MO_ERROR_NOT_EXECUTED);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    nodeP.uri = strdup(uri_bad);
    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context),
                     MO_ERROR_NOT_FOUND);
    free(nodeP.uri);
    nodeP.uri = strdup(uri_dl);
    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context),
                     MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(nodeP.uri);
    CU_ASSERT_FALSE( strcmp(nodeP.uri, uri_dl) );
    CU_ASSERT_TRUE( nodeP.data_size );
    CU_ASSERT_PTR_NOT_NULL( nodeP.data_buffer );
    CU_ASSERT_PTR_NOT_NULL(nodeP.type);
    CU_ASSERT_PTR_NOT_NULL(nodeP.format);

    if(nodeP.format)
        free (nodeP.format);
    if(nodeP.type)
        free(nodeP.type);
    if(nodeP.data_buffer)
        free(nodeP.data_buffer);
    if(nodeP.uri)
        free(nodeP.uri);

    CU_ASSERT_EQUAL( g_interface->getFunc(NULL, context),
                     //MO_ERROR_NONE);
                     MO_ERROR_COMMAND_FAILED);
    g_interface->closeFunc(context);
}

void test_case_set_func(void)
{
    DM_LOGI("SCOMO TEST: test_case_set_func");
    char uri[] = "./SCM/download/Package1";
    char uriBad[] = "./SCM/download/Package2";
    dmtree_node_t nodeP, nodeO;
    void *context = NULL;
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;

    memset(&nodeP, 0, sizeof(nodeP));
    memset(&nodeO, 0, sizeof(nodeO));
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uri, &type, context),
                     MO_ERROR_NONE);
    //                 MO_ERROR_NOT_EXECUTED);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    // invalid data test
    nodeP.uri = uri;
    nodeP.format = "chr";
    nodeP.type = "text/plain";
    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                     MO_ERROR_COMMAND_FAILED);

    nodeP.data_buffer = "severity";
    nodeP.data_size = strlen(nodeP.data_buffer)+1;
    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                     MO_ERROR_NONE);

    CU_ASSERT_EQUAL( g_interface->setFunc(NULL, context),
                     MO_ERROR_COMMAND_FAILED);

    // add new node
    nodeP.uri = uriBad;
    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                     MO_ERROR_NONE);

    g_interface->closeFunc(context);
}

void test_case_get_acl_func(void)
{
    DM_LOGI("SCOMO TEST: test_case_get_acl_func");

    void *context = NULL;
    char uriBad[] = "./SCM/Pkg";
    char uri[] = "./SCM/download/Package1";
    char *acl = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setACLFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->getACLFunc(0, &acl, context),
        MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->getACLFunc(uriBad, 0, context),
        MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->getACLFunc(uriBad, &acl, context),
        MO_ERROR_NOT_FOUND);

    CU_ASSERT_EQUAL( g_interface->getACLFunc(uri, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);

    if(acl)
        free(acl);

    g_interface->closeFunc(context);

}

void test_case_set_acl_func(void)
{
    DM_LOGI("SCOMO TEST: test_case_set_acl_func");
    void *context = NULL;
    char uriBad[] = "./SCM/Pkg";
    char acl[] = "acl";

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setACLFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->setACLFunc(0, acl, context),
        MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->setACLFunc(uriBad, 0, context),
        MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->setACLFunc(uriBad, acl, context),
        MO_ERROR_NOT_FOUND);

    CU_ASSERT_EQUAL( g_interface->setACLFunc(SCM_BASE_URI, acl, context),
            MO_ERROR_NONE);

    g_interface->closeFunc(context);
}

void test_case_exec_func(void)
{
    DM_LOGI("SCOMO TEST: test_case_exec_func");
    char uriBad[] = "./SCM/Pkg";
    char uriGood[] = "./SCM/download/Package1";

    char cmdData[] = "cmdData";
    char correlator[] = "correlator";
    void *context = NULL;
   // void *threadRC = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->execFunc);

    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->execFunc(0, cmdData, correlator,
                        context),
                    MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->execFunc(uriBad, cmdData, correlator,
                        context),
                    MO_ERROR_NOT_EXECUTED);

    CU_ASSERT_NOT_EQUAL( g_interface->execFunc(uriGood, cmdData, correlator,
                                               context), MO_ERROR_NONE);

    scm_node node;
    node.acl_= "Get=*&Replace=*&Exec=*";
    node.node_kind_= OMADM_NODE_IS_LEAF;;
    node.format_ = efnf_Char;
    node.node_command_ability_= 7;
    node.value_.data_ = "peration_data";
    node.value_.size_= strlen(node.value_.data_);
    char * uri1 = "./SCM/download/Package1/Operations/Install";
    CU_ASSERT_EQUAL(scm_set_node("./SCM/download/Package1/Operations",&node),
                     MO_ERROR_NONE);
    node.value_.data_ = "install_data";
    node.value_.size_= strlen(node.value_.data_);
    CU_ASSERT_EQUAL(scm_set_node(uri1,&node), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/download/Package1/PkgID",&node),
                    MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/download/Package1/PkgURL",&node),
                    MO_ERROR_NONE);

    CU_ASSERT_EQUAL( g_interface->execFunc(uri1, cmdData, correlator,
                                               context), MO_ERROR_NONE);

    g_interface->closeFunc(context);
}

int omadm_event_handler (omadm_mo_event_t *event)
{
    (void)event;
    return 0;
}

void test_case_event_handler()
{
    DM_LOGI("SCOMO TEST: test_case_event_handler");
    void *data = NULL;
    omadm_mo_ft_event_handler event_handler = NULL;
    CU_ASSERT_EQUAL(
        omadm_mo_register_event_handler(data, event_handler ), 1 );

    event_handler = omadm_event_handler;
    CU_ASSERT_EQUAL(
        omadm_mo_register_event_handler(data, event_handler ), 0 );

    event_handler = NULL;
    CU_ASSERT_EQUAL(
        omadm_mo_unregister_event_handler(data, event_handler ), 1 );

    event_handler = omadm_event_handler;
    CU_ASSERT_EQUAL(
        omadm_mo_unregister_event_handler(data, event_handler ), 0 );
}

void test_case_utility()
{
    DM_LOGI("SCOMO TEST: test_case_utility");

    char * node_format = NULL;
    char * node_type = NULL;
    scm_node_format format;
    scm_cleanup();
    format = efnf_Char;
    get_string_node_format(format, &node_format);
    CU_ASSERT_STRING_EQUAL(node_format, "chr");
    get_string_node_type(format,&node_type);
    CU_ASSERT_STRING_EQUAL(node_type,"text/plain");
    free(node_format);
    free(node_type);

    format = efnf_Node;
    get_string_node_format(format, &node_format);
    CU_ASSERT_STRING_EQUAL(node_format, "node");
    get_string_node_type(format,&node_type);
    CU_ASSERT_PTR_NULL(node_type);
    free(node_format);

    format = efnf_Bin;
    get_string_node_format(format, &node_format);
    CU_ASSERT_STRING_EQUAL(node_format, "bin");
    get_string_node_type(format,&node_type);
    CU_ASSERT_STRING_EQUAL(node_type,"text/plain");
    free(node_format);
    free(node_type);

    format = efnf_Int;
    get_string_node_format(format, &node_format);
    CU_ASSERT_STRING_EQUAL(node_format, "int");
    get_string_node_type(format,&node_type);
    CU_ASSERT_STRING_EQUAL(node_type,"text/plain");
    free(node_type);
    free(node_format);

    format = efnf_Bool;
    get_string_node_format(format, &node_format);
    CU_ASSERT_STRING_EQUAL(node_format, "bool");
    get_string_node_type(format,&node_type);
    CU_ASSERT_STRING_EQUAL(node_type,"text/plain");
    free(node_type);
    free(node_format);

    format = efnf_Unknown;
    get_string_node_format(format, &node_format);
    CU_ASSERT_STRING_EQUAL(node_format, "unknown");
    get_string_node_type(format,&node_type);
    CU_ASSERT_STRING_EQUAL(node_type,"text/plain");
    free(node_type);
    free(node_format);
}

void test_case_node_type()
{
     DM_LOGI("SCOMO TEST: test_case_node_type");
     scm_node node;
     memset(&node,0,sizeof(scm_node));

     char * nformat = OMADM_LEAF_FORMAT_CHAR;
     CU_ASSERT_EQUAL(get_node_type_from_string(&node.format_,
                                               &node.node_kind_,
                                               nformat),
                                               MO_ERROR_NONE);
     CU_ASSERT_EQUAL(node.format_, efnf_Char);
     CU_ASSERT_EQUAL(node.node_kind_, OMADM_NODE_IS_LEAF);
     nformat = OMADM_NODE_FORMAT;
     CU_ASSERT_EQUAL(get_node_type_from_string(&node.format_,
                                               &node.node_kind_,
                                               nformat),
                                               MO_ERROR_NONE);
     CU_ASSERT_EQUAL(node.format_, efnf_Node);
     CU_ASSERT_EQUAL(node.node_kind_, OMADM_NODE_IS_INTERIOR);
     nformat = OMADM_LEAF_FORMAT_BOOL;
     CU_ASSERT_EQUAL(get_node_type_from_string(&node.format_,
                                               &node.node_kind_,
                                               nformat),
                                               MO_ERROR_NONE);
     CU_ASSERT_EQUAL(node.format_, efnf_Bool);
     CU_ASSERT_EQUAL(node.node_kind_, OMADM_NODE_IS_LEAF);
     nformat = OMADM_LEAF_FORMAT_INT;
     CU_ASSERT_EQUAL(get_node_type_from_string(&node.format_,
                                               &node.node_kind_,
                                               nformat),
                                               MO_ERROR_NONE);
     CU_ASSERT_EQUAL(node.format_, efnf_Int);
     CU_ASSERT_EQUAL(node.node_kind_, OMADM_NODE_IS_LEAF);
     nformat = OMADM_LEAF_FORMAT_INT;
     CU_ASSERT_EQUAL(get_node_type_from_string(&node.format_,
                                               &node.node_kind_,
                                               nformat),
                                               MO_ERROR_NONE);
     CU_ASSERT_EQUAL(node.format_, efnf_Int);
     CU_ASSERT_EQUAL(node.node_kind_, OMADM_NODE_IS_LEAF);

     nformat = OMADM_LEAF_FORMAT_FLOAT;
     CU_ASSERT_EQUAL(get_node_type_from_string(&node.format_,
                                               &node.node_kind_,
                                               nformat),
                                               MO_ERROR_NONE);
     CU_ASSERT_EQUAL(node.format_, efnf_Char);
     CU_ASSERT_EQUAL(node.node_kind_, OMADM_NODE_IS_LEAF);

     nformat = "bad_format";
     CU_ASSERT_EQUAL(get_node_type_from_string(&node.format_,
                                               &node.node_kind_,
                                               nformat),
                                               MO_ERROR_NONE);
     CU_ASSERT_EQUAL(node.format_, efnf_Unknown);
     CU_ASSERT_EQUAL(node.node_kind_, OMADM_NODE_NOT_EXIST);

     CU_ASSERT_EQUAL(get_node_type_from_string(&node.format_,
                                               &node.node_kind_,
                                               NULL),
                                               MO_ERROR_COMMAND_FAILED);
}

void test_case_exec_precondition(void)
{
    DM_LOGI("SCOMO TEST: test_case_exec_precondition");
    char * uri1 = "./SCM/download/Package1/Operations/Install";
    char * uri2 = "./SCM/inventory/delivered/Package4/Operations/Install";
    char * uri3 = "./SCM/inventory/deployed/Package5/Operations/Activate";
    char *pname = NULL;
    char *parent_uri = NULL;
    char *command = NULL;
    scm_node node;
    node.acl_= "Get=*&Replace=*&Exec=*";
    node.node_kind_= OMADM_NODE_IS_INTERIOR;
    node.format_ = efnf_Node;
    node.node_command_ability_= 7;
    node.value_.data_ = "";
    node.value_.size_= strlen(node.value_.data_);

    CU_ASSERT_EQUAL(check_exec_pre_condition(NULL, &pname, &parent_uri, &command),
                    MO_ERROR_NOT_EXECUTED);

    // create nodes
    CU_ASSERT_EQUAL(scm_set_node("./SCM/download/Package1/Operations",&node),
                     MO_ERROR_NONE);

    CU_ASSERT_EQUAL(scm_set_node("./SCM/inventory/delivered/Package4",&node),
                     MO_ERROR_NONE);

    CU_ASSERT_EQUAL(scm_set_node("./SCM/inventory/delivered/Package4/Operations",
                                 &node), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/inventory/deployed/Package5",
                                 &node), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/inventory/deployed/Package5/Operations",
                                 &node), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/inventory/deployed/Package5/Operations/Activate",
                                 &node), MO_ERROR_NONE);

    node.node_kind_= OMADM_NODE_IS_LEAF;
    node.format_ = efnf_Char;
    node.value_.data_ = "operation";
    node.value_.size_= strlen(node.value_.data_);

    CU_ASSERT_EQUAL(scm_set_node(uri1, &node), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node(uri2, &node), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node(uri3, &node), MO_ERROR_NONE);
    // create
    CU_ASSERT_EQUAL(scm_set_node("./SCM/download/Package1/PkgID", &node),
                    MO_ERROR_NONE);

    CU_ASSERT_EQUAL(scm_set_node("./SCM/download/Package1/PkgURL", &node),
                    MO_ERROR_NONE);


    CU_ASSERT_EQUAL(check_exec_pre_condition(uri1, &pname, &parent_uri, &command),
                    MO_ERROR_NONE);

    if(pname) {
        free(pname);
        pname = NULL;
    }
    if(parent_uri){
        free(parent_uri);
        parent_uri = NULL;
    }
    if(command) {
        free(command);
        command = NULL;
    }


    // check download nodes
    CU_ASSERT_EQUAL(check_exec_pre_condition(uri1, &pname, &parent_uri, &command),
                    MO_ERROR_NONE);
    if(pname) {
        free(pname);
        pname = NULL;
    }
    if(parent_uri){
        free(parent_uri);
        parent_uri = NULL;
    }
    if(command) {
        free(command);
        command = NULL;
    }
    // check Deployed
    CU_ASSERT_EQUAL(scm_set_node("./SCM/inventory/delivered/Package4/PkgID", &node),
                    MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/inventory/delivered/Package4/Data", &node),
                    MO_ERROR_NONE);
    CU_ASSERT_EQUAL(check_exec_pre_condition(uri2, &pname, &parent_uri, &command),
                    MO_ERROR_NONE);
    if(pname) {
        free(pname);
        pname = NULL;
    }
    if(parent_uri){
        free(parent_uri);
        parent_uri = NULL;
    }
    if(command) {
        free(command);
        command = NULL;
    }
    // check Delivered
    CU_ASSERT_NOT_EQUAL(check_exec_pre_condition(uri3, &pname, &parent_uri, &command),
                    MO_ERROR_NONE);
    if(pname) {
        free(pname);
        pname = NULL;
    }
    if(parent_uri){
        free(parent_uri);
        parent_uri = NULL;
    }
    if(command) {
        free(command);
        command = NULL;
    }
    g_interface->closeFunc(NULL);
}

void sqlite_init(){
     CU_ASSERT_EQUAL(init_lib(),MO_ERROR_NONE);
     CU_ASSERT_EQUAL(init_database(),MO_ERROR_NONE);
}
void sqlite_get_node(void){
    scm_node *node;
    CU_ASSERT_EQUAL(scm_get_node("./SCM/download",&node ),MO_ERROR_NONE);
}

void sqlite_get_node_fail(void){
    scm_node *node;
    CU_ASSERT_EQUAL(scm_get_node("/download",&node ),MO_ERROR_NOT_FOUND);
    CU_ASSERT_EQUAL(close_database(),MO_ERROR_NONE);
}

void sqlite_test_get_prop(void){
    char *data;
    CU_ASSERT_EQUAL(scm_get_node_prop("./SCM/download",NODE_FORMAT, &data),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_get_node_prop("./SCM/download",NODE_KIND, &data),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_get_node_prop("./SCM/download",NODE_VALUE, &data),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_get_node_prop("./SCM/download",NODE_ABILITY, &data),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_get_node_prop("./SCM/download",NODE_ACL, &data),MO_ERROR_NONE);
}

void sqlite_test_get_prop_fail(void){
    char *data;
    CU_ASSERT_EQUAL(scm_get_node_prop("./SCM/download",8, &data),MO_ERROR_NOT_FOUND);
}

void sqlite_test_set_prop(void){
    char *data = "Replace=*";
    CU_ASSERT_EQUAL(scm_set_node_prop("./SCM/download",NODE_ACL, data),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node_prop("./SCM/download",NODE_FORMAT, "1"),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node_prop("./SCM/download",NODE_KIND, "1"),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node_prop("./SCM/download",NODE_ABILITY, "7"),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node_prop("./SCM/download",NODE_VALUE, "null"),MO_ERROR_NONE);
}

void sqlite_test_set_node(void){
    scm_node *node = calloc(1,sizeof(scm_node));
    node->acl_= "Replace=*";
    node->node_kind_= OMADM_NODE_IS_INTERIOR;
    node->format_ = efnf_Node;
    node->node_command_ability_= 7;
    node->value_.data_ = "some";
    node->value_.size_= strlen(node->value_.data_);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/download",node),MO_ERROR_NONE);
}

void sqlite_test_set_prop_fail(void){
    char *data = "Replace=*" ;
    CU_ASSERT_EQUAL(scm_set_node_prop("./SCM/download",8, data),MO_ERROR_COMMAND_FAILED);
}

void sqlite_test_set_node_add(void){
    scm_node *node = calloc(1,sizeof(scm_node));

    node->acl_= "Get=*";
    node->node_kind_= OMADM_NODE_IS_INTERIOR;
    node->format_ = efnf_Node;
    node->node_command_ability_= 7;
    node->value_.data_ = "";
    node->value_.size_= strlen(node->value_.data_);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/download/Pia",node),MO_ERROR_NONE);
}

void sqlite_node_with_same_name(void){
    scm_node *node = calloc(1,sizeof(scm_node));

    node->acl_= "Get=*";
    node->node_kind_= OMADM_NODE_IS_INTERIOR;
    node->format_ = efnf_Node;
    node->node_command_ability_= 7;
    node->value_.data_ = "";
    node->value_.size_= strlen(node->value_.data_);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/download/Pia",node),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(scm_set_node("./SCM/inventory/deployed/Pia",node),MO_ERROR_NONE);
}

void sqlite_push_job(void){
    char *pkg_name = "Package8";
    char *parent_uri = "download";
    char* command = "Install";
    char *cmd_data = "test_cmd_data";
    char *correlator = "test_correlator";
    CU_ASSERT_EQUAL(push_job_to_queue(pkg_name, parent_uri, command, cmd_data, correlator),MO_ERROR_NOT_FOUND);
    char *pkg_name1 = "Package4";
    char *parent_uri1 = "delivered";
    char* command1 = "Update";
    CU_ASSERT_EQUAL(push_job_to_queue(pkg_name1, parent_uri1, command1, cmd_data, correlator),MO_ERROR_NONE);
    char *pkg_name2 = "Package5";
    char *parent_uri2 = "deployed";
    char* command2 = "Activate";
    CU_ASSERT_EQUAL(push_job_to_queue(pkg_name2, parent_uri2, command2, cmd_data, correlator),MO_ERROR_NONE);
    char *pkg_name3 = "Package4";
    char *parent_uri3 = "download";
    char* command3 = "Install";
    char *cmd_data3 = NULL;
    char *correlator3 = NULL;
    CU_ASSERT_EQUAL(push_job_to_queue(pkg_name3, parent_uri3, command3, cmd_data3, correlator3),MO_ERROR_NONE);
}

void sqlite_pop_job(void){
    char* executed_node = "Activate";
    scm_job_description* job_des = NULL;
    int job_id = 0;
    get_job_from_queue(executed_node, &job_des);
    CU_ASSERT_PTR_NOT_NULL(job_des);
    if(job_des != NULL)
        job_id = job_des->job_key;

    CU_ASSERT_EQUAL(pop_job_from_queue(job_id),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(close_database(),MO_ERROR_NONE);
}

void sqlite_change_state_job(void){
    char* executed_node = "Install";
    scm_job_description* job_des = NULL;
    int job_id = 0;
    int state = 0;
    get_job_from_queue(executed_node, &job_des);
    job_id = job_des->job_key;
    state = job_des->state;

    CU_ASSERT_EQUAL(set_job_state_in_queue(job_id, state),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(get_job_from_queue(executed_node, &job_des),MO_ERROR_NONE);
}

void sqlite_get_childs(void){
    scm_node *node;
    CU_ASSERT_EQUAL(scm_get_node("./SCM/download",&node ),MO_ERROR_NONE);
    CU_ASSERT_EQUAL(get_childs("1",&node ),MO_ERROR_NONE);
    CU_ASSERT_STRING_EQUAL(node->value_.data_,"download/inventory");
}

void test_case_download_package(void)
{
    CU_ASSERT_EQUAL(scm_download_package(), 0);
}

void test_case_install_package(void)
{
    CU_ASSERT_EQUAL(scm_install_package(), 0);
}

void test_case_do_inactive(void)
{
    CU_ASSERT_EQUAL(scm_do_inactive(), 0);
}

void test_case_mutex_init_all(void)
{
    CU_ASSERT_EQUAL(mutex_init_all(), 0);
}

void test_case_mutex_destroy_all(void)
{
    CU_ASSERT_EQUAL(mutex_destroy_all(), 0);
}

void test_case_select_download_job(void)
{
    mutex_init_all();
    scm_job_description *info = calloc(1, sizeof(scm_job_description));
    CU_ASSERT_EQUAL(scm_select_download_job(info), 0);
    CU_ASSERT_EQUAL(info->pkg_primary_key, 0);
    CU_ASSERT_EQUAL(info->state, SCM_JOB_WAS_PUSH_TO_QUEUE);
    CU_ASSERT_EQUAL(info->job_key, 0);
    CU_ASSERT_EQUAL(0, strcmp("cmd_data", info->cmd_data));
    CU_ASSERT_EQUAL(0, strcmp("correlator",info->correlator));
    free(info);
    mutex_destroy_all();
}

void test_case_select_download_install_job(void)
{
    mutex_init_all();
    scm_job_description *info = calloc(1, sizeof(scm_job_description));
    CU_ASSERT_EQUAL(scm_select_download_install_job(info), 0);
    CU_ASSERT_EQUAL(info->pkg_primary_key, 0);
    CU_ASSERT_EQUAL(info->state, SCM_JOB_WAS_PUSH_TO_QUEUE);
    CU_ASSERT_EQUAL(info->job_key, 0);
    CU_ASSERT_EQUAL(0, strcmp("cmd_data", info->cmd_data));
    CU_ASSERT_EQUAL(0, strcmp("correlator",info->correlator));
    free(info);
    mutex_destroy_all();
}

void test_case_select_download_install_inactive_job(void)
{
    mutex_init_all();
    scm_job_description *info = calloc(1, sizeof(scm_job_description));
    CU_ASSERT_EQUAL(scm_select_download_install_inactive_job(info), 0);
    CU_ASSERT_EQUAL(info->pkg_primary_key, 0);
    CU_ASSERT_EQUAL(info->state, SCM_JOB_WAS_PUSH_TO_QUEUE);
    CU_ASSERT_EQUAL(info->job_key, 0);
    CU_ASSERT_EQUAL(0, strcmp("cmd_data", info->cmd_data));
    CU_ASSERT_EQUAL(0, strcmp("correlator",info->correlator));
    free(info);
    mutex_destroy_all();
}

void test_case_verify_conditions_ok(void)
{
    CU_ASSERT_EQUAL(0, scm_verify_conditions_ok(100,100));
    CU_ASSERT_EQUAL(1, scm_verify_conditions_ok(0,0));
}

void test_case_main_loop(void)
{
    CU_ASSERT_EQUAL(0, scm_main_loop());
}

/**
 * It adds tests suites and test cases for DMACC
 * @return 0 if success, 1 otherwise
 */
int main(){
    CU_pSuite pSuite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    pSuite = CU_add_suite("SCMSuite", init_suite, clean_suite);
    if(0 == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_omadm_get_mo_interface",
                test_case_omadm_get_mo_interface)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_init_func", test_case_init_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_close_func", test_case_close_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_is_node_func",
                test_case_is_node_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_find_urn_func",
                test_case_find_urn_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_get_func", test_case_get_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_set_func", test_case_set_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_get_acl_func",
                test_case_get_acl_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_set_acl_func",
                test_case_set_acl_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_exec_func", test_case_exec_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_event_handler",
                test_case_event_handler)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "scm_utility", test_case_utility)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(NULL == CU_add_test(pSuite, "exec_precondition",
                            test_case_exec_precondition)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "scm_node_type ",
             test_case_node_type)) {
         CU_cleanup_registry();
         return CU_get_error();
     }
    if(0 == CU_add_test(pSuite, "init",
                sqlite_init)) {
       CU_cleanup_registry();
       return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "get node",
                sqlite_get_node)) {
       CU_cleanup_registry();
       return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "get node fail",
            sqlite_get_node_fail)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "prop",
            sqlite_test_get_prop)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "prop fail",
            sqlite_test_get_prop_fail)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "set prop",
            sqlite_test_set_prop)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "set prop fail ",
            sqlite_test_set_prop_fail)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "set node ",
            sqlite_test_set_node)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "add node ",
            sqlite_test_set_node_add)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "add same node ",
            sqlite_node_with_same_name)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "add_job ",
            sqlite_push_job)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "change state ",
            sqlite_change_state_job)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "pop_job ",
            sqlite_pop_job)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if(0 == CU_add_test(pSuite, "sqlite_get_childs ",
            sqlite_get_childs)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "test_case_download_package",
                test_case_download_package)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "test_case_install_package",
                test_case_install_package)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_case_do_inactive",
            test_case_do_inactive)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_case_mutex_init_all",
                test_case_mutex_init_all)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_case_mutex_destroy_all",
                test_case_mutex_destroy_all)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_case_select_download_job",
            test_case_select_download_job)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_case_select_download_install_job",
            test_case_select_download_install_job)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_case_select_download_install_inactive_job",
            test_case_select_download_install_inactive_job)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_case_verify_conditions_ok",
            test_case_verify_conditions_ok)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_case_main_loop",
            test_case_main_loop)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

   /* Run all tests using the CUnit Basic interface */
       CU_basic_set_mode(CU_BRM_SILENT);
       CU_basic_run_tests();
       unsigned int failedTests = CU_get_number_of_tests_failed();
       if(failedTests)
       {
           printf("Failure list:");
           CU_basic_show_failures(CU_get_failure_list());
           printf("\n");
       }
       CU_ErrorCode errorResult = CU_get_error();
       CU_cleanup_registry();
       return errorResult == CUE_SUCCESS ? failedTests : errorResult;
}
