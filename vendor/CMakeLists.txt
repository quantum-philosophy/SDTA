add_custom_target (hotspot)

foreach (version ${HOTSPOT_VERSIONS})
  add_custom_target (hotspot-${version}
    COMMAND ${CMAKE_BUILD_TOOL} -C ${CMAKE_CURRENT_SOURCE_DIR}/hotspot-${version}
  )
  add_custom_command (
    TARGET hotspot-${version}
    POST_BUILD
    COMMAND ln -fs ${CMAKE_CURRENT_SOURCE_DIR}/hotspot-${version}/hotspot ${PROJECT_BINARY_DIR}/hotspot-${version}
  )
  add_dependencies (hotspot hotspot-${version})
endforeach (version)
