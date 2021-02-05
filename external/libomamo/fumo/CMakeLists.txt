# Copyright (C) 2016 Verizon. All Rights Reserved.

project(mofumo)
cmake_minimum_required(VERSION 2.8)

SET(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# POSIX thread supporting
if(NOT ${dmc_platform} STREQUAL "mn6")
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)
    if(CMAKE_USE_PTHREADS_INIT)
        set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${CMAKE_THREAD_LIBS_INIT})
    else(CMAKE_USE_PTHREADS_INIT)
        message(FATAL_ERROR "POSIX Threads were not supported")
    endif(CMAKE_USE_PTHREADS_INIT)
endif()

include_directories(src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../_include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/src)

get_filename_component(DATA_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
if(DATA_DIR MATCHES  /mo)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/cmake)
  include(config)
endif()

# path to proiduce all deployable binaries in common place
get_property(lib_output GLOBAL PROPERTY LIB_OUTPUT_DIR)

get_property(dmc_platform GLOBAL PROPERTY DMC_PLATFORM)
# do we still need this check?
if (NOT dmc_platform)
    set(dmc_platform "no")
endif ()

get_property(cur_data_location GLOBAL PROPERTY CUR_DATA_LOCATION)
get_property(pal_install_dir GLOBAL PROPERTY PAL_INSTALL_DIR)
get_property(pal_lib_name GLOBAL PROPERTY PAL_LIB_NAME)
get_property(mo_work_path GLOBAL PROPERTY MO_WORK_PATH)

add_definitions(-DPAL_INSTALL_DIR=\"${pal_install_dir}\")
add_definitions(-DPAL_LIB_NAME=\"${pal_lib_name}\")
add_definitions(-DMO_WORK_PATH=\"${mo_work_path}\")
add_definitions(-DCURRENT_DATA_LOCATION=\"${cur_data_location}\")
add_definitions(-DDMC_PLATFORM=\"${dmc_platform}\")
message("FUMO: PAL_INSTALL_DIR=${pal_install_dir}")
message("FUMO: PAL_LIB_NAME=${pal_lib_name}")
message("FUMO: MO_WORK_PATH=${mo_work_path}")
message("FUMO: DATA=${cur_data_location}")

if (ENABLE_TEST)
  SET(CMAKE_C_FLAGS_COVERAGE
      "-g -O0 -W -fprofile-arcs -ftest-coverage")
  include(utils)
endif()

if (${dmc_platform} STREQUAL "rp2")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE} -lrt")
else ()
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE}")
endif ()

set (CMAKE_MODULE_LINKER_FLAGS
        "${CMAKE_MODULE_LINKER_FLAGS} avoid-version -module -shared -export-dynamic -export-symbols-regex '^omadm_get_mo_interface' ${CMAKE_C_FLAGS_COVERAGE}")

include_directories( ../_common
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../_include)
set (SOURCES
        src/fumo.c
        src/fumo_node.c
        src/fumo_work_threads.c
        src/fumo_state_storage.c
        src/json.c)
add_library (${PROJECT_NAME} SHARED ${SOURCES})
set_property(TARGET ${PROJECT_NAME}
             PROPERTY LIBRARY_OUTPUT_DIRECTORY ${lib_output})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

if (ENABLE_TEST)
  enable_testing()

  #     fumo.c UTs
  set (test_SOURCES
          test/fumo_test.c)

  set(TEST_NAME test-${PROJECT_NAME})
  add_executable(${TEST_NAME} ${test_SOURCES})
  target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS} ${PROJECT_NAME}
                                    ${CMAKE_THREAD_LIBS_INIT})
  add_test (MY${TEST_NAME} ${TEST_NAME})
  message("exec path == ${pwd}/${TEST_NAME}")
  ADD_COVERAGE_TARGET(${PROJECT_NAME} ${TEST_NAME})

  #     fumo_node.c UTs
  set (test_node_SOURCES
          test/fumo_node_test.c)
  set(TEST_NODE_NAME test-node-${PROJECT_NAME})
  add_executable(${TEST_NODE_NAME} ${test_node_SOURCES})
  target_link_libraries(${TEST_NODE_NAME} ${DL_LIBS} ${CUNIT_LIBS}
                        ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
  add_test (MY${TEST_NODE_NAME} ${TEST_NODE_NAME})
  message("exec path == ${pwd}/${TEST_NODE_NAME}")
  ADD_COVERAGE_TARGET(${PROJECT_NAME}-node ${TEST_NODE_NAME})

  #     fumo_work_threads.c UTs
  set (test_threads_SOURCES
          test/fumo_work_threads_test.c)
  set(TEST_THREADS_NAME test-threads-${PROJECT_NAME})
  add_executable(${TEST_THREADS_NAME} ${test_threads_SOURCES})
  target_link_libraries(${TEST_THREADS_NAME} ${DL_LIBS} ${CUNIT_LIBS}
                        ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
  add_test (MY${TEST_THREADS_NAME} ${TEST_THREADS_NAME})
  message("exec path == ${pwd}/${TEST_THREADS_NAME}")
  ADD_COVERAGE_TARGET(${PROJECT_NAME}-threads ${TEST_NODE_NAME})

  #     fumo_state_storage_test.c UTs
  set (test_storage_SOURCES
          test/fumo_state_storage_test.c)
  set(TEST_STORAGE_NAME test-storage-${PROJECT_NAME})
  add_executable(${TEST_STORAGE_NAME} ${test_storage_SOURCES})
  target_link_libraries(${TEST_STORAGE_NAME} ${DL_LIBS} ${CUNIT_LIBS}
                        ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
  add_test (MY${TEST_STORAGE_NAME} ${TEST_STORAGE_NAME})
  message("exec path == ${pwd}/${TEST_STORAGE_NAME}")
  ADD_COVERAGE_TARGET(${PROJECT_NAME}-storage ${TEST_NODE_NAME})

  #     fumo functional tests
  set (test_functional_SOURCES
          test/fumo_functional_test.c)
  set(TEST_FUNCTIONAL_NAME test-functional-${PROJECT_NAME})
  add_executable(${TEST_FUNCTIONAL_NAME} ${test_functional_SOURCES})
  target_link_libraries(${TEST_FUNCTIONAL_NAME} ${DL_LIBS} ${CUNIT_LIBS}
                        ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
  add_test (MY${TEST_FUNCTIONAL_NAME} ${TEST_FUNCTIONAL_NAME})
  add_custom_command(TARGET ${TEST_FUNCTIONAL_NAME} POST_BUILD
    COMMAND
    cp  ${CMAKE_CURRENT_SOURCE_DIR}/test/download_descriptor_local.xml
        ${PROJECT_BINARY_DIR})
  add_custom_command(TARGET ${TEST_FUNCTIONAL_NAME} POST_BUILD
    COMMAND
    cp  ${CMAKE_CURRENT_SOURCE_DIR}/test/VZW_Model_123_456.pdf
        ${PROJECT_BINARY_DIR})
  message("exec path == ${pwd}/${TEST_FUNCTIONAL_NAME}")
  ADD_COVERAGE_TARGET(${PROJECT_NAME}-functional ${TEST_NODE_NAME})

endif()

## @todo remove hardcoded dependency to libdl
find_library(DL_LIBS dl PATHS /usr/lib/)
if(DL_LIBS)
  target_link_libraries(${PROJECT_NAME} ${DL_LIBS})
else()
  MESSAGE(FATAL_ERROR "libdl not found! Aborting...")
endif()

