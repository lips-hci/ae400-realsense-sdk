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

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
    ${CMAKE_SOURCE_DIR}/third-party/lips/lib/${OS}
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

if(${LIB_BACKEND_ETHERNET} STREQUAL "backend-ethernet-NOTFOUND")
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