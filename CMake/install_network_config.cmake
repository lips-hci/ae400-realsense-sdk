configure_file(config/network.json config/network.json @ONLY)

if(UNIX)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/config/network.json"
        DESTINATION /usr/etc/LIPS/lib
)
endif(UNIX)

if(WIN32)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/config/network.json"
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
add_custom_target(install_network_config
    DEPENDS ${LRS_TARGET}
    OUTPUT network.json
    SOURCES ${CMAKE_SOURCE_DIR}/config/network.json
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/config/network.json
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/network.json
)
endif(WIN32)
