# Copyright (C) 2016 Verizon. All Rights Reserved.

cmake_minimum_required(VERSION 2.8)
if(${CMAKE_SOURCE_DIR} MATCHES /mo)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../_utils/cmake)
  include(config)
endif()
#add dir(s) with target
add_subdirectory(_common)
add_subdirectory(connmo)
add_subdirectory(dcmo)
add_subdirectory(devdetail)
add_subdirectory(devinfo)
add_subdirectory(diagmon)
add_subdirectory(dmacc)
add_subdirectory(fumo)
add_subdirectory(scm)

if (ENABLE_TEST)
  enable_testing()
  include(utils)

  ADD_COVERAGE_TARGET(all-plugins ${CMAKE_CTEST_COMMAND})
endif()

