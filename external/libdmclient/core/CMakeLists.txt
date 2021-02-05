# Copyright (C) 2016 Verizon. All Rights Reserved.

project (vzwcore)
cmake_minimum_required(VERSION 2.8)

get_filename_component(DATA_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
message("### DATA_DIR == ${DATA_DIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../omadm/external/dmcore/include)

if(DATA_DIR MATCHES /client)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/cmake)
  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
                         ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/src
                         ${CMAKE_CURRENT_SOURCE_DIR}/../../_include
     )
#  hack to get access to directory outside of current one
#  set(common_utils_dir "../../_utils/src")
#  add_subdirectory(${common_utils_dir} ${CMAKE_BINARY_DIR}/_utils)
  include(config)
endif()

get_property(cur_data_location GLOBAL PROPERTY CUR_DATA_LOCATION)
get_property(pal_install_dir GLOBAL PROPERTY PAL_INSTALL_DIR)
get_property(pal_lib_name GLOBAL PROPERTY PAL_LIB_NAME)
add_definitions(-DCURRENT_DATA_LOCATION=\"${cur_data_location}\")
add_definitions(-DPAL_INSTALL_DIR=\"${pal_install_dir}\")
add_definitions(-DPAL_LIB_NAME=\"${pal_lib_name}\")

add_definitions(-DDMC_RELEASE_VER=\"${dmc_release_ver}\")

include(utils)

# POSIX thread supporting
if(NOT ${dmc_platform} STREQUAL "mn6")
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)
    if(CMAKE_USE_PTHREADS_INIT)
        set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${CMAKE_THREAD_LIBS_INIT})
    else(CMAKE_USE_PTHREADS_INIT)
        message(FATAL_ERROR "POSIX Threads are required but not found")
    endif(CMAKE_USE_PTHREADS_INIT)
endif()

add_subdirectory(src)

if (ENABLE_TEST)
    enable_testing()
    add_definitions(-DENABLE_TEST)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE}")
    add_subdirectory(test)
endif()

