# Copyright (C) 2016 Verizon. All Rights Reserved.

include(utils)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ${CMAKE_C_FLAGS_COVERAGE}")
set(main_binary_name dmclient)

set(SOURCES
        controller.c
        init.c
        plugin_manager.c
        dmc_queue.c
        net_manager.c
   )

set(INIT_SRC
        launcher.c)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

get_property(udm_lib_name
             GLOBAL
             PROPERTY UDM_LIB_NAME)

add_dependencies(${PROJECT_NAME} vzwomadm ${udm_lib_name})
target_link_libraries(${PROJECT_NAME} vzwomadm ${udm_lib_name})

add_executable(${main_binary_name} ${INIT_SRC} ${SOURCES})
set_property(TARGET ${main_binary_name}
             PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


include_directories(${CMAKE_INCLUDE_PATH})

target_link_libraries(${main_binary_name}
        ${PROJECT_NAME}
        ${DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
        vzwomadm
        ${udm_lib_name})
