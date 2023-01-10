# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseExamplesCaptureSafe)

if(BUILD_GRAPHICAL_EXAMPLES)
    add_executable(lips-capture-safe lips-capture-safe.cpp ../example.hpp)
    set_property(TARGET lips-capture-safe PROPERTY CXX_STANDARD 11)
    target_link_libraries(lips-capture-safe ${DEPENDENCIES})
    include_directories(../ ../../third-party/tclap/include ../../examples)
    set_target_properties (lips-capture-safe PROPERTIES FOLDER "Examples")

    install(TARGETS lips-capture-safe RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
