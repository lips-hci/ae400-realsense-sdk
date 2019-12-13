#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(LipsExamplesImuReader)

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

add_executable(lips_imu_reader lips_imu_reader.cpp)
target_link_libraries(lips_imu_reader ${DEPENDENCIES})
set_target_properties (lips_imu_reader PROPERTIES
FOLDER "Examples"
)

install(TARGETS lips_imu_reader RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})