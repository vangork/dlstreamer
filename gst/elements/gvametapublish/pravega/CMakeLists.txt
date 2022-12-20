#
# Copyright (c) Dell Inc., or its subsidiaries. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#

cmake_minimum_required(VERSION 3.1)

set (TARGET_NAME "gvametapublishpravega")

find_package(PkgConfig REQUIRED)
find_path(PRAVEGA_CLIENT_INCLUDE_DIR pravega_client.h /usr/include/ /usr/local/include /home/luis/projects/pravega-client-rust/target/include)
find_library(PRAVEGA_CLIENT_LIBRARY pravega_client /usr/lib/ /usr/local/lib/ /home/luis/projects/pravega-client-rust/target/release)
pkg_check_modules(GSTREAMER gstreamer-1.0>=1.16 REQUIRED)

file (GLOB MAIN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
file (GLOB MAIN_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

add_library(${TARGET_NAME} SHARED ${MAIN_SRC} ${MAIN_HEADERS})
set_compile_flags(${TARGET_NAME})

target_include_directories(${TARGET_NAME}
PRIVATE
    ${GLIB2_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${PRAVEGA_CLIENT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET_NAME}
PRIVATE
    ${GSTREAMER_LIBRARIES}
    ${PRAVEGA_CLIENT_LIBRARY}
    common
    gvametapublish
)

install(TARGETS ${TARGET_NAME} DESTINATION ${DLSTREAMER_PLUGINS_INSTALL_PATH})
