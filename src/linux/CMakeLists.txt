# FIND prebuilt static lib
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
endif(WIN32)

# default we use lib backend-ethernet v0.9 now
# RS2 API version 2.17~2.20 => lib backend-ethernet v0.9
# RS2 API version 2.21~2.3x => lib backend-ethernet v1.0
if(${REALSENSE_VERSION_STRING} VERSION_LESS 2.21)
    set(LIBBE_VERSION_MAJOR 0)
    set(LIBBE_VERSION_MINOR 9)
    # IMPORTANT!! MUST HAVE for patch!!
    # Explicitly link librealsense2 to LIBUSB
    target_link_libraries(${LRS_TARGET} PRIVATE ${LIBUSB1_LIBRARIES})
else()
    set(LIBBE_VERSION_MAJOR 1)
    set(LIBBE_VERSION_MINOR 0)

    # after RS2 API 2.21, libusb_config is included on demand
    include(libusb_config)
    target_link_libraries(${LRS_TARGET} PRIVATE usb)
endif()

if(USE_EXTERNAL_USB)
    add_dependencies(${LRS_TARGET} libusb)
endif()

set(LIBVERSION ${LIBBE_VERSION_MAJOR}.${LIBBE_VERSION_MINOR})

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
    message(FATAL_ERROR "backend-ethernet library not found! Please check github.com/lips-hci")
elseif(NOT TARGET backend-ethernet)
    add_library(backend-ethernet STATIC IMPORTED)
    set_target_properties(backend-ethernet PROPERTIES
        IMPORTED_LOCATION_RELEASE ${LIB_BACKEND_ETHERNET}
        IMPORTED_LOCATION_DEBUG ${LIB_BACKEND_ETHERNET_DEBUG}
    )
    message(STATUS "Found backend-ethernet library: ${LIB_BACKEND_ETHERNET}")
    target_link_libraries(${LRS_TARGET} PRIVATE backend-ethernet)
    mark_as_advanced(LIB_BACKEND_ETHERNET LIB_BACKEND_ETHERNET_DEBUG)
endif()
