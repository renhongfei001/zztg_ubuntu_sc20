# Copyright (C) 2016 Verizon. All Rights Reserved.

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

SET(rpi_tools $ENV{DMC_RPI_TOOLS})
if (NOT rpi_tools)
    SET(rpi_tools /export/build/_shared/raspberrypi-tools)
    message("set rpi_tools to ${rpi_tools}")
endif ()

get_property(platform GLOBAL PROPERTY DMC_PLATFORM)
if (NOT platform)
    set(platform rp2)
    message("set platform to ${platform}")
endif ()

SET(arch_deps $ENV{DMC_PLATFORM_DIR})
if (NOT arch_deps)
    set(arch_deps ${CMAKE_CURRENT_SOURCE_DIR}/platform/${platform})
    message("set arch_deps to ${arch_deps}")
endif ()

set(arch arm-linux-gnueabihf)
SET(compiler_bin ${rpi_tools}/arm-bcm2708/gcc-linaro-${arch}-raspbian-x64/bin)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH ${arch_deps}/_dev-fs)

set(CMAKE_C_COMPILER ${compiler_bin}/${arch}-gcc)
set(CMAKE_C_COMPILER_TARGET ${compiler_bin}/${arch})
set(CMAKE_CXX_COMPILER ${compiler_bin}/${arch}-g++)
set(CMAKE_CXX_COMPILER_TARGET ${compiler_bin}/${arch})

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

include_directories (BEFORE ${arch_deps}/_dev-fs/usr/include/)
include_directories (BEFORE ${arch_deps}/_dev-fs/usr/include/${arch}/)
