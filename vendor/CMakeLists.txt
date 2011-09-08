# HotSpot
set (HOTSPOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hotspot-${HOTSPOT_VERSION})
add_custom_target (hotspot
	COMMAND ${CMAKE_BUILD_TOOL} -C ${HOTSPOT_DIR}
)
add_custom_command (
	TARGET hotspot
	POST_BUILD
	COMMAND ln -fs ${HOTSPOT_DIR}/hotspot ${PROJECT_BINARY_DIR}/hotspot
	COMMAND ln -sfn ${CMAKE_CURRENT_SOURCE_DIR}/hotspot-${HOTSPOT_VERSION} ${CMAKE_CURRENT_SOURCE_DIR}/hotspot
)
add_custom_target (hotspot-patch
	COMMAND perl -pi -e 's/^EXTRAFLAGS.*/EXTRAFLAGS=-fPIC/' ${HOTSPOT_DIR}/Makefile
)
add_dependencies (hotspot hotspot-patch)

# TGFF
set (TGFF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tgff-${TGFF_VERSION})
add_custom_target (tgff
	COMMAND ${CMAKE_BUILD_TOOL} -C ${TGFF_DIR}
)
add_custom_command (
	TARGET tgff
	POST_BUILD
	COMMAND ln -fs ${TGFF_DIR}/tgff ${PROJECT_BINARY_DIR}/tgff
	COMMAND ln -sfn ${TGFF_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tgff
)

# ParadisEO
set (PARADISEO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/paradiseo-${PARADISEO_VERSION})
add_custom_target (paradiseo
	COMMAND cd ${PARADISEO_DIR}/paradiseo-eo/build && cmake ..
	-DCMAKE_CXX_FLAGS_RELEASE='-O3 -DNDEBUG -fPIC' && make
	COMMAND cd ${PARADISEO_DIR}/paradiseo-moeo/build && cmake ..
	-DCMAKE_CXX_FLAGS_RELEASE='-O3 -DNDEBUG -fPIC' && make
)
add_custom_command (
	TARGET paradiseo
	POST_BUILD
	COMMAND ln -sfn ${PARADISEO_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/paradiseo
)
