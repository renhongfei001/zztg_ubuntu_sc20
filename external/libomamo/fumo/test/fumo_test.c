/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "fumo.h"

omadm_mo_interface_t* g_interface = 0;

extern int gui_supported;

///////////////////////////////////////////// fumo_core
int init_suite(void)
{
    remove("./data/fumo.state.store");
    g_interface = omadm_get_mo_interface();
    return (g_interface != 0 ? 0 : 1);
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite(void)
{
    remove("./data/fumo.state.store");
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
    omadm_mo_interface_t* interface = omadm_get_mo_interface();
    CU_ASSERT_PTR_NOT_NULL_FATAL(interface);

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

    if(interface) {
        if(interface->base_uri) {
            free(interface->base_uri);
        }
        free(interface);
    }
};

void test_case_init_func(void)
{
    void *data = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);

    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL( g_interface->initFunc(&data),
        MO_ERROR_NONE );
    remove("./data/fumo.state.store");
};

void test_case_close_func(void)
{
    void *data = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);

    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL( g_interface->initFunc(&data),
        MO_ERROR_NONE );

    //void fumo_root_closeFN(void *data)
    g_interface->closeFunc(data);

    data = 0;
    g_interface->closeFunc(data);
    remove("./data/fumo.state.store");
};

void test_case_is_node_func(void)
{
    //int fumo_root_isNodeFN(const char *uri, omadmtree_node_kind_t *type,
    //                       void *data)
    char uriBad[] = "./FUMO/Pkg";
    char uriGood[] = "./ManagedObjects/FUMO";
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;
    void *data = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);

    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL( g_interface->initFunc(&data),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uriBad, 0, data),
                     MO_ERROR_COMMAND_FAILED);
    CU_ASSERT_EQUAL( g_interface->isNodeFunc(0, &type, data),
                     MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->isNodeFunc("", &type, data),
                     MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uriBad, &type, data),
                     MO_ERROR_NOT_FOUND);
    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uriGood, &type, data),
                     MO_ERROR_NONE);

    g_interface->closeFunc(data);
    remove("./data/fumo.state.store");
};

void test_case_find_urn_func(void)
{
    // int fumo_root_findUrnFN (const char * urn, char *** urlsP, void * data)
    char urn[] = FUMO_URN;
    char urnBad[] = "bad:urn";
    char **urlsP = 0, **urls = 0;
    void *context = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->findURNFunc);

    remove("./data/fumo.state.store");
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
        free(*urls);
        ++urls;
    }
    free(urlsP);

    g_interface->closeFunc(context);
    remove("./data/fumo.state.store");
};

void test_case_get_func(void)
{
    remove("./data");
    // int fumo_root_getFN (dmtree_node_t * nodeP, void * data)
    char uri[] = "./ManagedObjects/FUMO/Update/PkgData";
    char uriPkgURL[] = "./ManagedObjects/FUMO/Download/PkgURL";
    char uriDownload[] = "./ManagedObjects/FUMO/Download";
    char uriBad[] = "./FUMO/Pkg";
    dmtree_node_t nodeP;
    dmtree_node_t nodeOut;
    void *context = NULL;
    omadmtree_node_kind_t type = OMADM_NODE_NOT_EXIST;
    FILE *fd = NULL;

    char uriVersion[] = "./ManagedObjects/FUMO/PkgVersion";
    char uriName[] = "./ManagedObjects/FUMO/PkgName";
    char *nn = NULL;

    memset(&nodeP, 0, sizeof(nodeP));

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);

    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uri, &type, context),
                     MO_ERROR_NONE);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context),
                     MO_ERROR_COMMAND_FAILED);

    nodeP.uri = uriBad;
    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context),
                     MO_ERROR_NOT_FOUND);

    nodeP.uri = uri;
    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context),
                     MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(nodeP.uri);
    CU_ASSERT_FALSE( strcmp(nodeP.uri, uri) );
    CU_ASSERT_FALSE( nodeP.data_size );
    CU_ASSERT_PTR_NULL( nodeP.data_buffer );
    CU_ASSERT_PTR_NOT_NULL(nodeP.type);
    CU_ASSERT_PTR_NOT_NULL(nodeP.format);

    /** ./ManagedObjects/FUMO/PkgVersion testing */
    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uriVersion, &type, context),
                     MO_ERROR_NONE);
    nodeP.uri = uriVersion;

    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context), MO_ERROR_NONE);
    /** @todo make it right
    if(MO_ERROR_NOT_FOUND == check_pkg_version(&nn)) {
        CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context), MO_ERROR_NOT_FOUND);
    }

    CU_ASSERT_EQUAL( fumo_storage_set_pkg_version(uri), MO_ERROR_NONE );
    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context), MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(nodeP.data_buffer);
    CU_ASSERT_FALSE( strcmp(nodeP.data_buffer, uri) ); */

    /** ./ManagedObjects/FUMO/PkgName testing */
    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uriName, &type, context),
                     MO_ERROR_NONE);
    nodeP.uri = uriName;

    fd = fopen(MO_WORK_PATH"/data/"PAL_FILENAME_FIRMWARE_PACKET_NAME, "w");
    CU_ASSERT_PTR_NOT_NULL(fd);
    fwrite(uri, strlen(uri)+1, 1, fd);
    fclose(fd);

    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context), MO_ERROR_NONE);
    CU_ASSERT_FALSE( strcmp(nodeP.data_buffer, uri) );

    remove(MO_WORK_PATH"/data/"PAL_FILENAME_FIRMWARE_PACKET_NAME);
    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeP, context), MO_ERROR_COMMAND_FAILED);

    memset(&nodeP, 0, sizeof(nodeP));
    nodeP.uri = uriPkgURL;
    nodeP.data_buffer = "url number one";
    nodeP.data_size = strlen(nodeP.data_buffer);
    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                                     MO_ERROR_NONE);
    memset(&nodeOut, 0, sizeof(nodeOut));
    nodeOut.uri = uriPkgURL;
    CU_ASSERT_EQUAL(g_interface->getFunc(&nodeOut, context), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(nodeOut.data_size, strlen(nodeP.data_buffer));
    CU_ASSERT_EQUAL(memcmp(nodeOut.data_buffer, nodeP.data_buffer,
        strlen(nodeP.data_buffer)), 0);

    nodeP.uri = uriPkgURL;
    nodeP.data_buffer = "url small";
    nodeP.data_size = strlen(nodeP.data_buffer);
    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                                     MO_ERROR_NONE);
    memset(&nodeOut, 0, sizeof(nodeOut));
    nodeOut.uri = uriPkgURL;
    CU_ASSERT_EQUAL(g_interface->getFunc(&nodeOut, context), MO_ERROR_NONE);
    CU_ASSERT_EQUAL(nodeOut.data_size, strlen(nodeP.data_buffer));
    CU_ASSERT_EQUAL(memcmp(nodeOut.data_buffer, nodeP.data_buffer,
        strlen(nodeP.data_buffer)), 0);

    nodeP.uri = uriDownload;
    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeOut, context), MO_ERROR_NONE);

    g_interface->closeFunc(context);
    remove("./data/fumo.state.store");
};

void test_case_set_func(void)
{
    // int fumo_root_setFN (const dmtree_node_t * nodeP, void * data)
    char uri[] = "./ManagedObjects/FUMO/Ext/Severity";
    char uriBad[] = "./FUMO/Pkg";
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

    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->isNodeFunc(uri, &type, context),
                     MO_ERROR_NONE);
    CU_ASSERT_EQUAL(type, OMADM_NODE_IS_LEAF);

    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                     MO_ERROR_COMMAND_FAILED);

    nodeP.uri = uriBad;
    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                     MO_ERROR_COMMAND_NOT_IMPLEMENTED);

    nodeP.uri = uri;
    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                     MO_ERROR_NONE);

    nodeP.data_buffer = "severity";
    nodeP.data_size = strlen(uri)+1;
    CU_ASSERT_EQUAL( g_interface->setFunc(&nodeP, context),
                     MO_ERROR_NONE);
    nodeO.uri = uri;
    CU_ASSERT_EQUAL( g_interface->getFunc(&nodeO, context),
                     MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(nodeO.uri);
    CU_ASSERT_FALSE( strcmp(nodeO.uri, uri) );
    CU_ASSERT_EQUAL( nodeO.data_size, strlen("severity") );
    CU_ASSERT_PTR_NOT_NULL( nodeO.data_buffer );
    CU_ASSERT_FALSE( memcmp(nodeO.data_buffer, "severity", strlen("severity")) );
    CU_ASSERT_PTR_NOT_NULL(nodeO.type);
    CU_ASSERT_PTR_NOT_NULL(nodeO.format);

    g_interface->closeFunc(context);
    remove("./data/fumo.state.store");
};

void test_case_get_acl_func(void)
{
    // int fumo_root_getACLfn (const char * uri, char ** aclP, void * data)
    void *context = NULL;
    char uriBad[] = "./FUMO/Pkg";
    char *acl = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setACLFunc);

    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->getACLFunc(0, &acl, context),
        MO_ERROR_NONE);

    CU_ASSERT_EQUAL( g_interface->getACLFunc(uriBad, 0, context),
        MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->getACLFunc(uriBad, &acl, context),
        MO_ERROR_NOT_FOUND);
    CU_ASSERT_PTR_NULL(acl);

    /** @todo make it right
    CU_ASSERT_EQUAL( g_interface->getACLFunc(uri, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NULL(acl); */

    // NodeRoot
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // PkgName
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_PKGNAME, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_PKGNAME, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // PkgVersion
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_PKGVERSION, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_PKGVERSION, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // Download
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // DownloadPkgURL
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_PKGURL, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_PKGURL, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // DownloadCancel
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_CANCEL, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_CANCEL, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // Update
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // UpdatePkgData
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE_PKGDATA, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE_PKGDATA, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // UpdateCancel
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE_CANCEL, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE_CANCEL, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // DownloadAndUpdate
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // DownloadAndUpdatePkgURL
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL,
        uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL,
        &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // DownloadAndUpdateCancel
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL,
        uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL,
        &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // State
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_STATE, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_STATE, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // Cancel
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_CANCEL, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_CANCEL, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // Ext
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // ExtSeverity
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_SEVERITY, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_SEVERITY, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // ExtNIWifiOnly
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    // ExtNIWifiOnlyTimer
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLYTIMER, uriBad, context),
        MO_ERROR_NONE);
    CU_ASSERT_EQUAL( g_interface->getACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLYTIMER, &acl, context),
        MO_ERROR_NONE);
    CU_ASSERT_PTR_NOT_NULL(acl);
    CU_ASSERT_FALSE(strcmp(acl, uriBad));

    free(acl);

    g_interface->closeFunc(context);
    remove("./data/fumo.state.store");
};

void test_case_set_acl_func(void)
{
    // int fumo_root_setACLfn (const char * uri, const char *acl, void * data)
    void *context = NULL;
    char uriBad[] = "./FUMO/Pkg";
    char acl[] = "acl";

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->setACLFunc);

    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->setACLFunc(0, acl, context),
        MO_ERROR_NONE);

    CU_ASSERT_EQUAL( g_interface->setACLFunc(uriBad, 0, context),
        MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->setACLFunc(uriBad, acl, context),
        MO_ERROR_NOT_FOUND);

    // NodeRoot
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI, acl, context),
            MO_ERROR_NONE);

    // PkgName
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_PKGNAME, acl, context),
            MO_ERROR_NONE);
    // PkgVersion
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_PKGVERSION, acl, context),
            MO_ERROR_NONE);

    // Download
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD, acl, context),
            MO_ERROR_NONE);

    // DownloadPkgURL
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_PKGURL, acl, context),
            MO_ERROR_NONE);

    // DownloadCancel
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_CANCEL, acl, context),
            MO_ERROR_NONE);

    // Update
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE, acl, context),
            MO_ERROR_NONE);

    // UpdatePkgData
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE_PKGDATA, acl, context),
            MO_ERROR_NONE);

    // UpdateCancel
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_UPDATE_CANCEL, acl, context),
            MO_ERROR_NONE);

    // DownloadAndUpdate
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE, acl, context),
            MO_ERROR_NONE);

    // DownloadAndUpdatePkgURL
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_PKGURL,
            acl, context),
            MO_ERROR_NONE);

    // DownloadAndUpdateCancel
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_DOWNLOAD_AND_UPDATE_CANCEL,
            acl, context),
            MO_ERROR_NONE);

    // State
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_STATE, acl, context),
            MO_ERROR_NONE);

    // Cancel
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_CANCEL, acl, context),
            MO_ERROR_NONE);

    // Ext
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT, acl, context),
            MO_ERROR_NONE);

    // ExtSeverity
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_SEVERITY, acl, context),
            MO_ERROR_NONE);

    // ExtNIWifiOnly
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLY, acl, context),
            MO_ERROR_NONE);

    // ExtNIWifiOnlyTimer
    CU_ASSERT_EQUAL( g_interface->setACLFunc(FUMO_BASE_URI "/" FUMO_URI_EXT_NI_WIFIONLYTIMER, acl, context),
            MO_ERROR_NONE);

    g_interface->closeFunc(context);
    remove("./data/fumo.state.store");
};

void test_case_exec_func(void)
{
    // int fumo_root_execFN (const char * uri, const char * cmdData,
    //                       const char * correlator, void * data)
    char uriBad[] = "./FUMO/Pkg";
    char uriGood[] = "./ManagedObjects/FUMO/Download";
    char uriNotAllow[] = "./ManagedObjects/FUMO/Update/PkgData";
    char cmdData[] = "cmdData";
    char correlator[] = "correlator";
    void *context = NULL;
    void *threadRC = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->initFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->closeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->isNodeFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->getFunc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_interface->execFunc);

    remove("./data/fumo.state.store");
    CU_ASSERT_EQUAL( g_interface->initFunc(&context),
        MO_ERROR_NONE );

    CU_ASSERT_EQUAL( g_interface->execFunc(0, cmdData, correlator,
                        context),
                    MO_ERROR_COMMAND_FAILED);

    CU_ASSERT_EQUAL( g_interface->execFunc(uriBad, cmdData, correlator,
                        context),
                     MO_ERROR_NOT_FOUND);

    CU_ASSERT_EQUAL( g_interface->execFunc(uriNotAllow, cmdData, correlator,
                        context),
                     MO_ERROR_NOT_EXECUTED);

    CU_ASSERT_EQUAL( g_interface->execFunc(uriGood, cmdData, correlator,
                        context),
                     FUMO_RC_ACCEPTED_FOR_PROCESSING);

    CU_ASSERT_FALSE( pthread_join(g_fumo_work_thread_id, &threadRC) );
    g_fumo_work_thread_id = 0;

    CU_ASSERT_EQUAL( threadRC, (void*)0 );

    g_interface->closeFunc(context);
    remove("./data/fumo.state.store");
};

int omadm_event_handler (omadm_mo_event_t *event)
{
    return 0;
}

void test_case_register_event_handler()
{
    void *data = NULL;
    omadm_mo_ft_event_handler event_handler = NULL;
    CU_ASSERT_EQUAL(
        omadm_mo_register_event_handler(data, event_handler ), 1 );

    event_handler = omadm_event_handler;
    CU_ASSERT_EQUAL(
        omadm_mo_register_event_handler(data, event_handler ), 0 );
}


void test_case_unregister_event_handler()
{
    void *data = NULL;
    omadm_mo_ft_event_handler event_handler = NULL;
    CU_ASSERT_EQUAL(
        omadm_mo_unregister_event_handler(data, event_handler ), 1 );

    event_handler = omadm_event_handler;
    CU_ASSERT_EQUAL(
        omadm_mo_unregister_event_handler(data, event_handler ), 0 );
}

void test_case_load_pal_functions(void)
{
    const char *good_list[] = {
            "pal_update_firmware",
            "pal_update_firmware_cancel",
            "pal_download_firmware",
            "pal_download_cancel",
            "pal_system_fwv_get",
            "pal_system_batteries_get",
            "pal_system_battery_x_level_get",
            NULL };
    const char *bad_list[] = {"this_function_does_not_exists", NULL};

    void *pal_handle = NULL;
    CU_ASSERT_EQUAL(1, load_pal_functions(&pal_handle, NULL));

    pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pal_handle);
    CU_ASSERT_EQUAL(0, load_pal_functions(&pal_handle, good_list));
    dlclose(pal_handle);

    pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pal_handle);
    CU_ASSERT_EQUAL(1, load_pal_functions(&pal_handle, bad_list));
    dlclose(pal_handle);
}

void test_case_load_pal_gui_functions(void)
{
    const char *good_list[] = {
            "pal_fumo_gui_create_message",
            "pal_fumo_gui_destroy_message",
            "pal_fumo_gui_set_system_update_info",
            "pal_fumo_gui_update_download_progress",
            NULL };
    const char *bad_list[] = {"this_function_does_not_exists", NULL};
    int buf = gui_supported;

    void *pal_handle = NULL;
    CU_ASSERT_EQUAL(1, load_pal_gui_functions(&pal_handle, NULL));

    gui_supported = buf;

    pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pal_handle);
    CU_ASSERT_EQUAL(0, load_pal_gui_functions(&pal_handle, good_list));
    CU_ASSERT_EQUAL(0, gui_supported);
    dlclose(pal_handle);

    gui_supported = buf;//restore if previous test failed

    pal_handle = dlopen(PAL_INSTALL_DIR "/" PAL_LIB_NAME, RTLD_LAZY);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pal_handle);
    CU_ASSERT_EQUAL(0, load_pal_gui_functions(&pal_handle, bad_list));
    CU_ASSERT_EQUAL(0, gui_supported);
    dlclose(pal_handle);

    gui_supported = buf;//restore because test failed
}

void test_case_get_pal_func(void)
{
    void *pal_handle = NULL;
    int (*pal_fumo_init_fc)(void) = NULL;

    CU_ASSERT_EQUAL(NULL, get_pal_func(&pal_handle, NULL));
    CU_ASSERT_EQUAL(NULL, get_pal_func(&pal_handle, "this_function_does_not_exists"));

    pal_fumo_init_fc = get_pal_func(&pal_handle, "pal_fumo_init");
    CU_ASSERT_PTR_NOT_NULL_FATAL(pal_fumo_init_fc);

    if(pal_handle) dlclose(pal_handle);
}

int main()
{
    CU_pSuite suite = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    suite = CU_add_suite("fumo_core_suite", init_suite, clean_suite);
    if(0 == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_omadm_get_mo_interface",
            test_case_omadm_get_mo_interface)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_init_func",
            test_case_init_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_close_func",
            test_case_close_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_is_node_func",
            test_case_is_node_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_find_urn_func",
            test_case_find_urn_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_get_func",
            test_case_get_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_set_func",
            test_case_set_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_get_acl_func",
            test_case_get_acl_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_set_acl_func",
            test_case_set_acl_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_exec_func",
            test_case_exec_func)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_register_event_handler",
            test_case_register_event_handler)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "fumo_unregister_event_handler",
            test_case_unregister_event_handler)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_load_pal_functions",
            test_case_load_pal_functions)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_load_pal_gui_functions",
            test_case_load_pal_gui_functions)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(0 == CU_add_test(suite, "test_case_get_pal_func",
            test_case_get_pal_func)) {
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

