configure_file(config/network.json config/network.json @ONLY)

if(UNIX)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/config/network.json"
        DESTINATION /usr/etc/LIPS/lib
)
endif(UNIX)

if(WIN32)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/config/network.json"
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
)
add_custom_target(install_network_config ALL
    SOURCES config/network.json
    COMMAND ${CMAKE_COMMAND} -E copy
        "config/network.json"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/network.json"
)
add_dependencies(${LRS_TARGET} install_network_config)
endif(WIN32)
