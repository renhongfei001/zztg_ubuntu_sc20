# Copyright (C) 2016 Verizon. All Rights Reserved.

project (modevdetail)
cmake_minimum_required(VERSION 2.8)

SET(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

get_filename_component(DATA_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
if(DATA_DIR MATCHES  /mo)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/cmake)
  include(config)
endif()

get_property(lib_output GLOBAL PROPERTY LIB_OUTPUT_DIR)

get_property(pal_lib_name GLOBAL PROPERTY PAL_LIB_NAME)
get_property(pal_install_dir GLOBAL PROPERTY PAL_INSTALL_DIR)

add_definitions(-DPAL_INSTALL_DIR=\"${pal_install_dir}\")
add_definitions(-DPAL_LIB_NAME=\"${pal_lib_name}\")

if (ENABLE_TEST)
  SET(CMAKE_C_FLAGS_COVERAGE
      "-g -O0 -W -fprofile-arcs -ftest-coverage")
  include(utils)
endif()

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE}")

include_directories( ../_common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../_include
                    ${CMAKE_INCLUDE_PATH})
set (SOURCES 
        src/devdetail.c
        ../_common/plugin_utils.c)

#type MODULE for dynamic loaded library
add_library (${PROJECT_NAME} MODULE ${SOURCES})
set_property(TARGET ${PROJECT_NAME}
             PROPERTY LIBRARY_OUTPUT_DIRECTORY ${lib_output})

if (ENABLE_TEST)
  set (UTILS ../_common/plugin_utils.c)
  set (test_SOURCES
          test/devdetail_test.c
          ${UTILS})

  enable_testing()
  add_library (${PROJECT_NAME}_test SHARED ${SOURCES})
  set(TEST_NAME test-${PROJECT_NAME})
  add_executable(${TEST_NAME} ${test_SOURCES})
  target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS} ${PROJECT_NAME}_test)
  add_test (MY${TEST_NAME} ${TEST_NAME})
  message("exec path == ${pwd}/${TEST_NAME}")

  ADD_COVERAGE_TARGET(${PROJECT_NAME} ${TEST_NAME})
  endif()

