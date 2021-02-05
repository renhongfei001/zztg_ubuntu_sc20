# Copyright (C) 2016 Verizon. All Rights Reserved.

get_property(udm_lib_name
             GLOBAL
             PROPERTY UDM_LIB_NAME)

project(${udm_lib_name})
cmake_minimum_required(VERSION 2.8)

get_filename_component(DATA_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
message("### DATA_DIR == ${DATA_DIR}")
if(DATA_DIR MATCHES /client)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
  include(config)
endif()

if (ENABLE_TEST)
  SET(CMAKE_C_FLAGS_COVERAGE
      "-g -O0 -W -fprofile-arcs -ftest-coverage")
  include(utils)
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE}")

set(SOURCES
        src/udm_controller.c
        src/udm_interface_handler.c)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

message("lib done")

if (ENABLE_TEST)
#controller
  set(test_SOURCES
        test/udm_controller_test.c)
  set(TEST_NAME test-controller-${PROJECT_NAME})
  add_executable(${TEST_NAME} ${test_SOURCES})
  message("exec done")
  target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS} ${PROJECT_NAME})

  enable_testing()

  add_test(MY${TEST_NAME} ${TEST_NAME})
  message("exec path == ${pwd}/${TEST_NAME}")

  ADD_COVERAGE_TARGET(${PROJECT_NAME}-controller ${TEST_NAME})

#interface_handler
set(test_SOURCES
      test/udm_interface_handler_test.c)
set(TEST_NAME test-interface_handler-${PROJECT_NAME})
add_executable(${TEST_NAME} ${test_SOURCES})
message("exec done")
target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS} ${PROJECT_NAME})

enable_testing()

add_test(MY${TEST_NAME} ${TEST_NAME})
message("exec path == ${pwd}/${TEST_NAME}")

ADD_COVERAGE_TARGET(${PROJECT_NAME}-interface_handler ${TEST_NAME})
endif()
