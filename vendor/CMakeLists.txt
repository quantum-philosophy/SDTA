# Hotspot
set (hotspot_dir ${CMAKE_CURRENT_SOURCE_DIR}/hotspot-${HOTSPOT_VERSION})
add_custom_target (hotspot
	COMMAND ${CMAKE_BUILD_TOOL} -C ${hotspot_dir}
)
add_custom_command (
	TARGET hotspot
	POST_BUILD
	COMMAND ln -fs ${hotspot_dir}/hotspot ${CMAKE_CURRENT_BINARY_DIR}/hotspot
	COMMAND ln -sfn ${CMAKE_CURRENT_SOURCE_DIR}/hotspot-${HOTSPOT_VERSION} ${CMAKE_CURRENT_SOURCE_DIR}/hotspot
)

# TGFF
set (tgff_dir ${CMAKE_CURRENT_SOURCE_DIR}/tgff-${TGFF_VERSION})
add_custom_target (tgff
	COMMAND ${CMAKE_BUILD_TOOL} -C ${tgff_dir}
)
add_custom_command (
	TARGET tgff
	POST_BUILD
	COMMAND ln -fs ${tgff_dir}/tgff ${CMAKE_CURRENT_BINARY_DIR}/tgff
	COMMAND ln -sfn ${tgff_dir} ${CMAKE_CURRENT_SOURCE_DIR}/tgff
)

# ParadisEO
set (paradiseo_dir ${CMAKE_CURRENT_SOURCE_DIR}/paradiseo-${PARADISEO_VERSION})
add_custom_target (paradiseo
	COMMAND cd ${paradiseo_dir}/paradiseo-eo/build &&
    cmake .. -Dconfig=${paradiseo_dir}/install.cmake &&
    make
	COMMAND cd ${paradiseo_dir}/paradiseo-mo/build &&
    cmake .. -Dconfig=${paradiseo_dir}/install.cmake &&
    make
	COMMAND cd ${paradiseo_dir}/paradiseo-moeo/build &&
    cmake .. -Dconfig=${paradiseo_dir}/install.cmake &&
    make
)
add_custom_command (
	TARGET paradiseo
	POST_BUILD
	COMMAND ln -sfn ${paradiseo_dir} ${CMAKE_CURRENT_SOURCE_DIR}/paradiseo
)
