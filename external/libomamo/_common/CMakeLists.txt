# Copyright (C) 2016 Verizon. All Rights Reserved.

cmake_minimum_required(VERSION 2.8)

set(config_path "${CMAKE_CURRENT_SOURCE_DIR}/data/")
set(config_name "/dmacc-init.txt")
set(config_build_path ${CMAKE_BINARY_DIR}/data)
set(config_build_init_path ${CMAKE_BINARY_DIR}/init)
add_custom_target(prepare_config ALL
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${config_build_path}
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${config_build_init_path}
                  COMMAND cp ${config_path}${config_name} ${config_build_init_path}/${config_name}
                  COMMENT "copying ${config_name} to ${config_build_init_path}"

                  SOURCES ${config_path}${config_name})

if (ENABLE_TEST)
  set(current_data_file_name "/dmacc-current.txt")
  add_definitions(-DINIT_DATA_LOCATION=\"${config_build_init_path}${config_name}\")
  add_definitions(-DCURRENT_DATA_LOCATION=\"${config_build_path}\")
  add_definitions(-DCURRENT_DATA_FILE_NAME=\"${current_data_file_name}\")
  get_filename_component(DATA_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
  if(DATA_DIR MATCHES  /mo)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/cmake)
    include(config)
  endif()

  SET(CMAKE_C_FLAGS_COVERAGE
      "-g -O0 -W -fprofile-arcs -ftest-coverage")
  include(utils)

  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE}")

include_directories( ../_common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../_include
                    ${CMAKE_INCLUDE_PATH})

  set (UTILS plugin_utils.c)
  set (test_SOURCES
          test/plugin_utils_test.c
          ${UTILS})

  enable_testing()

  set(TEST_NAME test-common)
  add_executable(${TEST_NAME} ${test_SOURCES})
  target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS})
  # target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS})
  add_test (MY${TEST_NAME} ${TEST_NAME})
  message("exec path == ${pwd}/${TEST_NAME}")

  ADD_COVERAGE_TARGET(${PROJECT_NAME} ${TEST_NAME})
endif()
