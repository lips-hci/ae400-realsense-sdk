#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(LipsAE400Toolkit)

if(WIN32)
    # windows version is still developing
    return()
endif()

add_executable(lips_ae400_toolkit
    lips_ae400_toolkit.cpp
    linux_scan.h
    linux_scan.cpp
)

set_property(TARGET lips_ae400_toolkit PROPERTY CXX_STANDARD 11)
set_target_properties (lips_ae400_toolkit PROPERTIES FOLDER "Tools")

install(TARGETS lips_ae400_toolkit RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
