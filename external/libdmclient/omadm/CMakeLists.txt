# Copyright (C) 2016 Verizon. All Rights Reserved.

project (vzwomadm)
cmake_minimum_required(VERSION 2.8)

get_filename_component(DATA_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
message("### DATA_DIR == ${DATA_DIR}")
if(DATA_DIR MATCHES /client)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/cmake)
  include(config)
endif()

get_property(lib_output GLOBAL PROPERTY LIB_OUTPUT_DIR)

get_property(pal_lib_name GLOBAL PROPERTY PAL_LIB_NAME)
get_property(pal_install_dir GLOBAL PROPERTY PAL_INSTALL_DIR)

get_property(dmc_platform GLOBAL PROPERTY DMC_PLATFORM)
# do we still need this check?
if (NOT dmc_platform)
    set(dmc_platform "no")
endif ()

if (${dmc_platform} STREQUAL "mn6")
  set(mo_install_dir "/system/vendor/verizon/dmclient/lib")
elseif (${dmc_platform} STREQUAL "rp2")
  set(mo_install_dir "/usr/share/dmclient/lib")
else ()
  set(mo_install_dir ${lib_output} )
endif ()

add_definitions(-DMO_INSTALL_DIR=\"${mo_install_dir}\")
add_definitions(-DPAL_INSTALL_DIR=\"${pal_install_dir}\")
add_definitions(-DPAL_LIB_NAME=\"${pal_lib_name}\")

set (omadm_mockup_dir "${CMAKE_CURRENT_SOURCE_DIR}/_mockup/src")
add_definitions(-DOMADM_MOCKUP_DIR=\"${omadm_mockup_dir}\")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../_include)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/dmcore/include)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/dmcore/src)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/SyncMLRTK/src)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/SyncMLRTK/src/sml/inc)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/SyncMLRTK/src/sml/inc/win)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/SyncMLRTK/src/sml/mgr/inc)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../system/core/include)
# TODO: verify if still needed
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/src)
# TODO: verify if still needed
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../core/src)

include(utils)

add_subdirectory(external)
add_subdirectory(_mockup)

get_property(core_lib_name
             GLOBAL
             PROPERTY CORE_LIB_NAME
)

# magic is to put this line after all external & non-coverageable components!!!
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE}")

set(SOURCES
        src/omadm_controller.c
        src/omadm_event_handler.c
        src/omadm_interface_handler.c )

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_dependencies(${PROJECT_NAME} ${core_lib_name})

message("lib done")

if (ENABLE_TEST)
  add_dependencies(${PROJECT_NAME} omadmmock)
  target_link_libraries(${PROJECT_NAME} ${DL_LIBS} ${core_lib_name} omadmmock)
  set(test_SOURCES
        test/omadm_controller_test.c)

  set(TEST_NAME test-controller-${PROJECT_NAME})
  set(MOCK_SOURSES
        src/omadm_controller.c
        src/omadm_event_handler.c
        _mockup/src/omadm_interface_handler.c )

  add_library(${PROJECT_NAME}_test STATIC ${MOCK_SOURSES} )

#  target_link_libraries(${PROJECT_NAME}_test ${DL_LIBS})
  target_link_libraries(${PROJECT_NAME}_test ${core_lib_name} omadmmock ${DL_LIBS})

  add_executable(${TEST_NAME} ${test_SOURCES})
  message("exec done")
  target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS}
        ${PROJECT_NAME}_test)

  enable_testing()

  add_test(MY${TEST_NAME} ${TEST_NAME})
  message("exec path == ${pwd}/${TEST_NAME}")

  ADD_COVERAGE_TARGET(${PROJECT_NAME}-controller ${TEST_NAME})

 #event_handler
 set(test_SOURCES
       test/omadm_event_handler_test.c)
 set(TEST_NAME test-event_handler-${PROJECT_NAME})
 add_executable(${TEST_NAME} ${test_SOURCES})
 message("exec done")
 target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS} ${PROJECT_NAME})

 enable_testing()

 add_test(MY${TEST_NAME} ${TEST_NAME})
 message("exec path == ${pwd}/${TEST_NAME}")

 ADD_COVERAGE_TARGET(${PROJECT_NAME}-event_handler ${TEST_NAME})

 #interface_handler
 set(test_SOURCES
       test/omadm_interface_handler_test.c)
 set(TEST_NAME test-interface_handler-${PROJECT_NAME})
 add_executable(${TEST_NAME} ${test_SOURCES})
 message("exec done")
 target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS} ${PROJECT_NAME})

 enable_testing()

 add_test(MY${TEST_NAME} ${TEST_NAME})
 message("exec path == ${pwd}/${TEST_NAME}")

 ADD_COVERAGE_TARGET(${PROJECT_NAME}-interface_handler ${TEST_NAME})
else(ENABLE_TEST)
    target_link_libraries(${PROJECT_NAME} ${DL_LIBS} ${core_lib_name} vzwcore)

endif()
