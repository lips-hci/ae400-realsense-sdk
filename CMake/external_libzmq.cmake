message(STATUS "Use external libzmq")
include(ExternalProject)

if(WIN32)
    ExternalProject_Add(
        libzmq

        GIT_REPOSITORY "https://github.com/zeromq/libzmq.git"
        GIT_TAG "v4.3.2"

        UPDATE_COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/third-party/libzmq/CMakeLists.txt
                ${CMAKE_CURRENT_BINARY_DIR}/third-party/libzmq/CMakeLists.txt
        PATCH_COMMAND ""

        SOURCE_DIR "third-party/libzmq/"
        CMAKE_ARGS -DCMAKE_CXX_STANDARD_LIBRARIES=${CMAKE_CXX_STANDARD_LIBRARIES}
                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libzmq_install
                -DZMQ_BUILD_TESTS=OFF
                -DCMAKE_CXX_FLAGS_RELEASE=/MT
                -DCMAKE_CXX_FLAGS_DEBUG=/MTd
                -DPOLLER=epoll
        TEST_COMMAND ""
    )
else()
    ExternalProject_Add(
        libzmq

        GIT_REPOSITORY "https://github.com/zeromq/libzmq.git"
        GIT_TAG "v4.3.2"

        UPDATE_COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/third-party/libzmq/CMakeLists.txt
                ${CMAKE_CURRENT_BINARY_DIR}/third-party/libzmq/CMakeLists.txt
        PATCH_COMMAND ""

        SOURCE_DIR "third-party/libzmq/"
        CMAKE_ARGS -DCMAKE_CXX_STANDARD_LIBRARIES=${CMAKE_CXX_STANDARD_LIBRARIES}
                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libzmq_install
                -DZMQ_BUILD_TESTS=OFF
        TEST_COMMAND ""
    )
endif()
set(LIBZMQ_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libzmq_install/lib)
set(LIBZMQ_LOCAL_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/libzmq_install/include)

