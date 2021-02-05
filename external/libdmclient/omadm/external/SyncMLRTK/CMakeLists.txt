cmake_minimum_required(VERSION 2.8)

#SyncMLRTK

set(syncML_INCLUDE_DIR
     src
     src/sml/inc
     src/sml/inc/win
     src/sml/lib/inc
     src/sml/mgr/inc
     src/sml/wsm/inc
     src/sml/xlt/all
     src/sml/xlt/inc
     src/sml/lib/inc )
include_directories(${syncML_INCLUDE_DIR})

set(syncML_SOURCE
     src/sml/lib/all/liblock.c
     src/sml/lib/all/libmem.c
     src/sml/lib/all/libstr.c
     src/sml/lib/all/libutil.c
     src/sml/mgr/all/mgr.c
     src/sml/mgr/all/mgrcmdbuilder.c
     src/sml/mgr/all/mgrcmddispatcher.c
     src/sml/mgr/all/mgrinstancelist.c
     src/sml/mgr/all/mgrinstancemgr.c
     src/sml/mgr/all/mgrutil.c
     src/sml/xlt/all/xltdec.c
     src/sml/xlt/all/xltdecwbxml.c
     src/sml/xlt/all/xltdecxml.c
     src/sml/xlt/all/xltdevinf.c
     src/sml/xlt/all/xltenc.c
     src/sml/xlt/all/xltenccom.c
     src/sml/xlt/all/xltencwbxml.c
     src/sml/xlt/all/xltencxml.c
     src/sml/xlt/all/xltmetinf.c
     src/sml/xlt/all/xlttags.c
     src/sml/xlt/all/xltutilstack.c)

add_library(syncml ${syncML_SOURCE})
set_target_properties(syncml PROPERTIES POSITION_INDEPENDENT_CODE ON)
