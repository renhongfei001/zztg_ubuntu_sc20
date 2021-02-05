# Copyright (C) 2016 Verizon. All Rights Reserved.

cmake_minimum_required(VERSION 2.8)

if(DATA_DIR MATCHES /client)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/cmake)
  include(config)
endif()

set (INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../_include)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")

include_directories(${INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../external/dmcore/include)

set(HEADERS ${INCLUDE_DIR}/pal.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/omadm_interface_handler.h)

set(SOURCES
    src/omadm_interface_handler.c
    src/dmc_queue.c
    src/net_manager.c
    )

add_library(omadmmock STATIC ${SOURCES})
target_link_libraries(omadmmock vzwomadm)

message("lib done")
