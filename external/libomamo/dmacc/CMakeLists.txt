# Copyright (C) 2016 Verizon. All Rights Reserved.

project(modmacc)
cmake_minimum_required(VERSION 2.8)

SET(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

get_filename_component(DATA_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
if(DATA_DIR MATCHES  /mo)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../_utils/cmake)
  include(config)
endif()

# path to proiduce all deployable binaries in common place
get_property(lib_output GLOBAL PROPERTY LIB_OUTPUT_DIR)

set(data_init_output ${CMAKE_BINARY_DIR}/init)
set(init_data_file_name "/dmacc-init.txt")
set(current_data_file_name "/dmacc-current.txt")

get_property(pal_lib_name GLOBAL PROPERTY PAL_LIB_NAME)
get_property(pal_install_dir GLOBAL PROPERTY PAL_INSTALL_DIR)

get_property(dmc_platform GLOBAL PROPERTY DMC_PLATFORM)
# do we still need this check?
if (NOT dmc_platform)
    set(dmc_platform "no")
endif ()

if (${dmc_platform} STREQUAL "mn6")
  set(init_data_location "/system/vendor/verizon/dmclient/init")
elseif (${dmc_platform} STREQUAL "rp2")
  set(init_data_location "/usr/share/dmclient/init")
else ()
  set(init_data_location ${data_init_output})
endif ()

get_property(cur_data_location GLOBAL PROPERTY CUR_DATA_LOCATION)

add_definitions(-DPAL_INSTALL_DIR=\"${pal_install_dir}\")
add_definitions(-DPAL_LIB_NAME=\"${pal_lib_name}\")
add_definitions(-DINIT_DATA_LOCATION=\"${init_data_location}${init_data_file_name}\")
add_definitions(-DCURRENT_DATA_LOCATION=\"${cur_data_location}\")
add_definitions(-DCURRENT_DATA_FILE_NAME=\"${current_data_file_name}\")
add_definitions(-DDMC_PLATFORM=\"${dmc_platform}\")


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
        src/dmacc.c
        ../_common/plugin_utils.c
        )

#type MODULE for dynamic loaded library
add_library (${PROJECT_NAME} MODULE ${SOURCES})
set_property(TARGET ${PROJECT_NAME}
             PROPERTY LIBRARY_OUTPUT_DIRECTORY ${lib_output})

if (ENABLE_TEST)
  set (UTILS ../_common/plugin_utils.c)
  set (test_SOURCES
          test/dmacc_test.c
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

