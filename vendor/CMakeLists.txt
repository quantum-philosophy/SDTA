# HotSpot
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

add_custom_command (
  TARGET hotspot-${HOTSPOT_DEFAULT_VERSION}
  POST_BUILD
  COMMAND ln -sfn ${CMAKE_CURRENT_SOURCE_DIR}/hotspot-${HOTSPOT_DEFAULT_VERSION} ${CMAKE_CURRENT_SOURCE_DIR}/hotspot
)

# TGFF
set (TGFF_VERSION 3.5)
set (tgff_dir ${CMAKE_CURRENT_SOURCE_DIR}/tgff-${TGFF_VERSION})
add_custom_target (tgff
  COMMAND ${CMAKE_BUILD_TOOL} -C ${tgff_dir}
)
add_custom_command (
  TARGET tgff
  POST_BUILD
  COMMAND ln -fs ${tgff_dir}/tgff ${PROJECT_BINARY_DIR}/tgff
  COMMAND ln -sfn ${tgff_dir} ${CMAKE_CURRENT_SOURCE_DIR}/tgff
)

# Paradiseo
set (PARADISEO_VERSION 1.2.1)
set (paradiseo_dir ${CMAKE_CURRENT_SOURCE_DIR}/paradiseo-${PARADISEO_VERSION})
add_custom_target (paradiseo
  COMMAND cd ${paradiseo_dir} && source ./installParadiseo.sh
)
add_custom_command (
  TARGET paradiseo
  POST_BUILD
  COMMAND ln -sfn ${paradiseo_dir} ${CMAKE_CURRENT_SOURCE_DIR}/paradiseo
)
