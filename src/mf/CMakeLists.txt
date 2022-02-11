# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
# Copyright(c) 2020-2021 LIPS Corporation. All Rights Reserved.
# Export IMU get API to our librealsense
target_sources(${LRS_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/ae400-imu.cpp"
)

if(WIN32)
    set(OS win)
    # Variable TARGET_ARCH can be x86, x64
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bits
        set(TARGET_ARCH x64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32 bits
        set(TARGET_ARCH x86)
    endif()
else()
    set(OS linux)
    # Variable TARGET_ARCH can be amd64, i386, armhf, arm64
    execute_process(COMMAND bash -c "dpkg --print-architecture"
        OUTPUT_VARIABLE TARGET_ARCH
        ERROR_QUIET
    )
    string(STRIP ${TARGET_ARCH} TARGET_ARCH)
endif()

if(${REALSENSE_VERSION_STRING} VERSION_LESS 2.43)
    message(FATAL_ERROR "AE400 SDK requires RealSense SDK codebase v2.43 or later version")
else()
    set(LIBVERSION 1.0.2.0)
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
    ${CMAKE_SOURCE_DIR}/third-party/lips/lib/v${LIBVERSION}/${OS}
)

# Look for the library for Release
find_library(LIB_BACKEND_ETHERNET
    NAMES backend-ethernet
    PATH_SUFFIXES ${TARGET_ARCH} ${TARGET_ARCH}/Release
)

# Look for the library for Debug
find_library(LIB_BACKEND_ETHERNET_DEBUG
    NAMES backend-ethernet
    PATH_SUFFIXES ${TARGET_ARCH} ${TARGET_ARCH}/Debug
)

if(${LIB_BACKEND_ETHERNET} STREQUAL "LIB_BACKEND_ETHERNET-NOTFOUND")
    message(FATAL_ERROR "[LIPS PATCH] backend-ethernet library not found! Please check github.com/lips-hci")
else()
    if(NOT TARGET backend-ethernet)
        add_library(backend-ethernet STATIC IMPORTED)
    endif()
    set_target_properties(backend-ethernet PROPERTIES
        IMPORTED_LOCATION ${LIB_BACKEND_ETHERNET}
        IMPORTED_LOCATION_RELEASE ${LIB_BACKEND_ETHERNET}
        IMPORTED_LOCATION_MINSIZEREL ${LIB_BACKEND_ETHERNET}
        IMPORTED_LOCATION_RELWITHDEBINFO ${LIB_BACKEND_ETHERNET}
        IMPORTED_LOCATION_DEBUG ${LIB_BACKEND_ETHERNET_DEBUG}
    )
    message(STATUS "[LIPS PATCH] Found backend-ethernet library: ${LIB_BACKEND_ETHERNET}")
    target_link_libraries(${LRS_TARGET} PRIVATE backend-ethernet)
    mark_as_advanced(LIB_BACKEND_ETHERNET LIB_BACKEND_ETHERNET_DEBUG)
endif()

if(${REALSENSE_VERSION_STRING} VERSION_LESS 2.34)
message(STATUS "[LIPS PATCH] Disable global time by patching source timestamp_reader")
# Manually patch src/global_timestamp_reader.cpp: get_device_time_ms() to redefine hardware time ms
# get_device_time_ms() since v2.23
else()
message(STATUS "[LIPS PATCH] Disable global time by defining RASPBERRY_PI")
# Refer this commit to DISABLE global time reading (ds5_device::get_device_time_ms)
# depth streaming is easily stuck (when AE is on) due to device time reading on
# network transition, it can cause lot of fps performance impact.
#
#From 43294e992be3b84f164e0e917f6fa0adad9f50e0 Mon Sep 17 00:00:00 2001
#From: dorodnic <sergey.dorodnicov@intel.com>
#Date: Thu, 26 Mar 2020 03:52:54 -0700
#Subject: [PATCH] Disable global time on RPi
target_compile_definitions(${LRS_TARGET} PRIVATE RASPBERRY_PI)

endif()
