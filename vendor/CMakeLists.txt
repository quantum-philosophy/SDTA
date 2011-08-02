add_custom_target (hotspot)

foreach (version ${HOTSPOT_VERSIONS})
  set (dir ${CMAKE_CURRENT_SOURCE_DIR}/hotspot-${version})
  add_custom_target (hotspot-${version}
    COMMAND ${CMAKE_BUILD_TOOL} -C ${dir}
  )
  add_custom_target (patch-${version}
    COMMAND perl -pi -e 's/^EXTRAFLAGS.*/EXTRAFLAGS=-fPIC/' ${dir}/Makefile
  )
  add_custom_command (
    TARGET hotspot-${version}
    POST_BUILD
    COMMAND ln -fs ${dir}/hotspot ${PROJECT_BINARY_DIR}/hotspot-${version}
  )
  add_dependencies (hotspot-${version} patch-${version})
  add_dependencies (hotspot hotspot-${version})
endforeach (version)
