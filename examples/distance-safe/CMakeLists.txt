# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseExamplesMrecrash)

if(BUILD_GRAPHICAL_EXAMPLES)
    add_executable(lips-distance-safe lips-distance-safe.cpp)
    set_property(TARGET lips-distance-safe PROPERTY CXX_STANDARD 11)
    target_link_libraries(lips-distance-safe ${DEPENDENCIES})
    include_directories(../)
    set_target_properties (lips-distance-safe PROPERTIES FOLDER "Examples")

    install(TARGETS lips-distance-safe RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
