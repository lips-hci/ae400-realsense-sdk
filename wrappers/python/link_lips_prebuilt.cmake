# Remove src because we link to prebuilt backend-ethernet lib
if(${BACKEND} STREQUAL RS2_USE_V4L2_BACKEND)
    list(APPEND RAW_RS
        ../../src/linux/ae400-imu.cpp
    )
    set(REMOVE_RAW_RS
        ../../src/linux/backend-v4l2.cpp
        ../../src/linux/backend-hid.cpp
    )
endif()

if(${BACKEND} STREQUAL RS2_USE_WMF_BACKEND)
    list(APPEND RAW_RS
        ../../src/mf/ae400-imu.cpp
    )
    set(REMOVE_RAW_RS
        ../../src/mf/mf-uvc.cpp
        ../../src/mf/mf-hid.cpp
        ../../src/mf/mf-backend.cpp
    )
endif()

# HACK: introduce definition to fix LNK2005:GUID_DEVINTERFACE_USB_DEVICE
# already defined in wih-helpers.obj error, it conflicts to usb.lib(windows_winusb.obj
#if(WIN32 AND USE_EXTERNAL_USB)
#    target_compile_definitions(pybackend2 PRIVATE WITH_TRACKING=1)
#endif()

target_compile_definitions(pybackend2 PRIVATE ENABLE_AE400_IMU)

if(DEFINED RAW_RS)
    list(REMOVE_ITEM RAW_RS ${REMOVE_RAW_RS})
endif()

# Replace the pybackend2 source CPP by linking to our prebuilt lib
if(TARGET pybackend2)
    target_link_libraries(pybackend2 PRIVATE backend-ethernet)

    get_target_property(PYBIND_SRC pybackend2 SOURCES)

    list(REMOVE_ITEM PYBIND_SRC ${REMOVE_RAW_RS})

    set_target_properties(pybackend2 PROPERTIES
        SOURCES "${PYBIND_SRC}"
    )
endif()