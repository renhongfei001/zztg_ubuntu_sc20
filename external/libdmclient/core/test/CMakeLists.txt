# Copyright (C) 2016 Verizon. All Rights Reserved.

include_directories(${CMAKE_INCLUDE_PATH})
include_directories(${PROJECT_SOURCE_DIR}/src)

# common list of libraries used to link all test targets,
# simply redefine this variable in case if you need custom libs
set(test_Libs ${CUNIT_LIBS} ${PROJECT_NAME} ${DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} vzwomadm)

#init
set(init_test_SOURCES
    init_test.c
    ../src/init.c
   )
set(init_test_ID ${PROJECT_NAME}-init)
ADD_TEST_TARGET(${init_test_ID} init_test_SOURCES test_Libs)

#controller
set(cntrlr_test_SOURCES
    controller_test.c)
set(test_ID ${PROJECT_NAME}-controller)
ADD_TEST_TARGET(${test_ID} cntrlr_test_SOURCES test_Libs)

#plugin_manager
set(plg_mgr_test_SOURCES
    plugin_manager_test.c)
set(test_ID ${PROJECT_NAME}-plugin_manager)
ADD_TEST_TARGET(${test_ID} plg_mgr_test_SOURCES test_Libs)

#events_queue
set(queue_test_SOURCES
    dmc_queue_test.c)
set(test_ID ${PROJECT_NAME}-queue)
ADD_TEST_TARGET(${test_ID} queue_test_SOURCES test_Libs)

#network manager
set(network_manager_SOURCES
    net_manager_test.c)
set(test_ID ${PROJECT_NAME}-network_manager)
ADD_TEST_TARGET(${test_ID} network_manager_SOURCES test_Libs)


