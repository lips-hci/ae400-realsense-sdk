# FIND prebuilt static lib
if(WIN32)
    set(OS "win")
    # Variable TARGET_ARCH can be x86, x64
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bits
        set(TARGET_ARCH "x64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32 bits
        set(TARGET_ARCH "x86")
    endif()
else()
    set(OS "linux")
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

# For linux arch amd64, i386, armhf, arm64, .a are provided
# For win x86, x64 Release .lib are provided (No Debug lib)
find_library(LIB_BACKEND_ETHERNET
    NAMES backend-ethernet
    PATH_SUFFIXES ${TARGET_ARCH} ${TARGET_ARCH}/Release
)

if(${LIB_BACKEND_ETHERNET} STREQUAL "LIB_BACKEND_ETHERNET-NOTFOUND")
    message(FATAL_ERROR "backend-ethernet library not found! Please check github.com/lips-hci")
else()
    message(STATUS "backend-ethernet library is found at ${LIB_BACKEND_ETHERNET}")
    #Link to prebuilt static lib
    target_link_libraries(${LRS_TARGET} PRIVATE ${LIB_BACKEND_ETHERNET})
endif()