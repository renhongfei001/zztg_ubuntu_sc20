# Copyright (C) 2016 Verizon. All Rights Reserved.

project(moscm)
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

get_filename_component(DATA_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
if(DATA_DIR MATCHES  /mo)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/cmake)
  include(config)
endif()

find_library(DL_LIBS dl PATHS /usr/lib/)
if(NOT DL_LIBS)
  MESSAGE(FATAL_ERROR "libdl not found! Aborting...")
endif()

get_property(dmc_platform GLOBAL PROPERTY DMC_PLATFORM)
# do we still need this check?
if (NOT dmc_platform)
    set(dmc_platform "no")
endif ()

get_property(lib_output GLOBAL PROPERTY LIB_OUTPUT_DIR)

get_property(cur_data_location GLOBAL PROPERTY CUR_DATA_LOCATION)
get_property(pal_install_dir GLOBAL PROPERTY PAL_INSTALL_DIR)
get_property(pal_lib_name GLOBAL PROPERTY PAL_LIB_NAME)
get_property(sqlite_lib_dir GLOBAL PROPERTY SQLITE_LIB_DIR)
get_property(sqlite_lib_name GLOBAL PROPERTY SQLITE_LIB_NAME)

set (data_name "scm_nodes.db")
add_definitions(-DDATA_BASE_LOCALION=\"${cur_data_location}\")
add_definitions(-DDATA_BASE_NAME=\"${data_name}\")
add_definitions(-DPAL_INSTALL_DIR=\"${pal_install_dir}\")
add_definitions(-DPAL_LIB_NAME=\"${pal_lib_name}\")
add_definitions(-DSQLITE_LIB_DIR=\"${sqlite_lib_dir}\")
add_definitions(-DSQLITE_LIB_NAME=\"${sqlite_lib_name}\")

if (ENABLE_TEST)
  SET(CMAKE_C_FLAGS_COVERAGE
      "-g -O0 -W -fprofile-arcs -ftest-coverage")
  include(utils)
endif()

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE}")

set (CMAKE_MODULE_LINKER_FLAGS
        "${CMAKE_MODULE_LINKER_FLAGS} avoid-version -module -shared -export-dynamic -export-symbols-regex '^omadm_get_mo_interface' ${CMAKE_C_FLAGS_COVERAGE}")

include_directories( src
                     ../_common
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/src
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../_include
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../client/omadm/external)

set (SOURCES
        src/scm.c
        ../_common/plugin_utils.c
        src/scm_mock.c
        src/scm_data.c
        src/scm_job.c
        src/scm_download.c
        src/scm_install.c
        src/scm_inactive.c)
add_library (${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${DL_LIBS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET ${PROJECT_NAME}
             PROPERTY LIBRARY_OUTPUT_DIRECTORY ${lib_output})

if (ENABLE_TEST)
  set (test_SOURCES
          test/scm_test.c
)

  enable_testing()

  set(TEST_NAME test-${PROJECT_NAME})
  add_executable(${TEST_NAME} ${test_SOURCES})
  target_link_libraries(${TEST_NAME} ${DL_LIBS} ${CUNIT_LIBS} ${PROJECT_NAME})
  add_test (MY${TEST_NAME} ${TEST_NAME})
  message("exec path == ${pwd}/${TEST_NAME}")

  ADD_COVERAGE_TARGET(${PROJECT_NAME} ${TEST_NAME})

  #     test scm downlod
  set (test_dl_SOURCES
          test/scm_test_dl.c)
  set(TEST_DL_NAME test-dl-${PROJECT_NAME})
  add_executable(${TEST_DL_NAME} ${test_dl_SOURCES})
  target_link_libraries(${TEST_DL_NAME} ${DL_LIBS} ${CUNIT_LIBS}
                        ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
  add_test (MY${TEST_DL_NAME} ${TEST_DL_NAME})
  message("exec path == ${pwd}/${TEST_DL_NAME}")
  ADD_COVERAGE_TARGET(${PROJECT_NAME}-dl ${TEST_DL_NAME})


endif()
