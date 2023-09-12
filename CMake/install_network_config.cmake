configure_file(config/network.json config/network.json @ONLY)

# '/usr/etc/LIPS/lib' is legacy way to put network config into system path
# the problem is permission denied when copying file
# Now we want to go with local config way, put it along with realsense tools/examples
# so at runtime executables can find it to read IP address of LIPSedge cameras
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/config/network.json"
        DESTINATION ${CMAKE_INSTALL_BINDIR}/
)

# for Windows env., copy network config to CONFIG folder:Release or Debug
if(WIN32)
add_custom_target(install_network_config ALL
    SOURCES config/network.json
    COMMAND ${CMAKE_COMMAND} -E copy
        "config/network.json"
        "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/network.json"
)
add_dependencies(${LRS_TARGET} install_network_config)
endif(WIN32)
