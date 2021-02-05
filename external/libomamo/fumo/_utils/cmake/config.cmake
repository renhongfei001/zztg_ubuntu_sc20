# Copyright (C) 2016 Verizon. All Rights Reserved.

cmake_minimum_required(VERSION 2.8)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source: e.g. use command `cmake -Bbuild -H.` to create `build` directory and do out-of-source build there.")
endif()

set(dmc_platform $ENV{DMC_PLATFORM})
if (NOT dmc_platform)
    set(dmc_platform "no")
endif ()

set_property(GLOBAL PROPERTY DMC_PLATFORM ${dmc_platform})

#global property to proiduce all deployable binaries in common place
set_property(GLOBAL PROPERTY LIB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/lib)

if (${dmc_platform} STREQUAL "mn6")
    set_property(GLOBAL PROPERTY PAL_LIB_NAME "libpal.so")
    set_property(GLOBAL PROPERTY PAL_INSTALL_DIR "/system/lib")
    set_property(GLOBAL PROPERTY MO_WORK_PATH "/data/vendor/verizon/dmclient")
    set_property(GLOBAL PROPERTY SQLITE_LIB_DIR "/system/lib")
    set_property(GLOBAL PROPERTY SQLITE_LIB_NAME "libsqlite.so")
    set_property(GLOBAL PROPERTY CUR_DATA_LOCATION "/data/vendor/verizon/dmclient/data")
    add_definitions(-DFUMO_UI_PLATFORM)
elseif (${dmc_platform} STREQUAL "rp2")
    set_property(GLOBAL PROPERTY PAL_LIB_NAME "libpal.so")
    set_property(GLOBAL PROPERTY PAL_INSTALL_DIR "/usr/share/dmclient/lib")
    set_property(GLOBAL PROPERTY SQLITE_LIB_DIR "/usr/lib/arm-linux-gnueabihf")
    set_property(GLOBAL PROPERTY SQLITE_LIB_NAME "libsqlite3.so.0")
    set_property(GLOBAL PROPERTY MO_WORK_PATH "/usr/share/dmclient")
    set_property(GLOBAL PROPERTY CUR_DATA_LOCATION "/usr/share/dmclient/data")
    add_definitions(-DFUMO_DCD_PLATFORM)
else ()
    set_property(GLOBAL PROPERTY PAL_LIB_NAME "libvzwmockup.so")
    get_property(lib_output GLOBAL PROPERTY LIB_OUTPUT_DIR)
    set_property(GLOBAL PROPERTY PAL_INSTALL_DIR ${lib_output})
    set_property(GLOBAL PROPERTY SQLITE_LIB_DIR "/usr/local/lib")
    set_property(GLOBAL PROPERTY SQLITE_LIB_NAME "libsqlite3.so")
    set_property(GLOBAL PROPERTY MO_WORK_PATH ".")
    set_property(GLOBAL PROPERTY CUR_DATA_LOCATION ${CMAKE_BINARY_DIR}/data)
    add_definitions(-DFUMO_DCD_PLATFORM)
endif ()

set(dmc_release_ver $ENV{DMC_RELEASE_VER})
if (NOT dmc_release_ver)
    set(dmc_release_ver "NA")
endif ()

