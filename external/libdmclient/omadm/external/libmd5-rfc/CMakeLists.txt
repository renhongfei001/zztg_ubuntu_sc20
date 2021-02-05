cmake_minimum_required(VERSION 2.8)
include_directories(libmd5-rfc)
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}  -g ")
#md5 lib
add_library(md5rfc md5.h md5.c )
set_target_properties(md5rfc PROPERTIES POSITION_INDEPENDENT_CODE ON)
