cmake_minimum_required(VERSION 2.8)

set (CMAKE_MODULE_LINKER_FLAGS
        "${CMAKE_MODULE_LINKER_FLAGS} -export-symbols-regex '^omadmclient_|^dmtree_node_'")
include_directories(../libmd5-rfc)

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}  -g ")

set(syncML_INCLUDE_DIR
     ../SyncMLRTK/src
     ../SyncMLRTK/src/sml/inc
     ../SyncMLRTK/src/sml/inc/win
     ../SyncMLRTK/src/sml/lib/inc
     ../SyncMLRTK/src/sml/mgr/inc
     ../SyncMLRTK/src/sml/wsm/inc
     ../SyncMLRTK/src/sml/xlt/all
     ../SyncMLRTK/src/sml/xlt/inc
     ../SyncMLRTK/src/sml/lib/inc )

include_directories(${syncML_INCLUDE_DIR})

include_directories(include)

##using name 'dmcore' to not interfere with main binary target
set_property(GLOBAL
             PROPERTY CORE_LIB_NAME dmcore
)

get_property(core_lib_name
             GLOBAL
             PROPERTY CORE_LIB_NAME
)

set(dmcore_HEADERS include/omadmclient.h
     include/omadmtree_mo.h
     include/syncml_error.h)
set(dmcore_SOURCES src/omadmclient.c
     src/callbacks.c
     src/codec.c
     src/package0.c
     src/credentials.c
     src/sml2tree.c
     src/utils.c
     src/momgr.c
     src/dmtree.c
     src/uricheck.c
     src/defaultroot.c
     src/log.c)
add_library(${core_lib_name} ${dmcore_HEADERS} ${dmcore_SOURCES})

add_dependencies(${core_lib_name} md5rfc syncml)
# This property determines whether position independent executables or shared
# libraries will be created.  False for static library by default.
# Only position independent code is supposed to be included into shared objects.
set_target_properties(${core_lib_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${core_lib_name} syncml md5rfc)
