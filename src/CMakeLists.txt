# Library
add_definitions (-Wall -Wno-write-strings -O2 -mfpmath=sse -msse3 -pg -DDEBUG)
set (hotspot_SRCS
	${PROJECT_SOURCE_DIR}/vendor/hotspot/flp.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/temperature.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/temperature_block.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/temperature_grid.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/RCutil.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/util.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/npe.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/shape.c
  external/hotspot.c
)
include_directories (
  ${PROJECT_SOURCE_DIR}/vendor
)
set_source_files_properties (${hotspot_SRCS} PROPERTIES LANGUAGE CXX)
add_library (hotspot STATIC ${hotspot_SRCS})
set_target_properties (hotspot PROPERTIES COMPILE_FLAGS "-fPIC")

# MatLab
set (MEX_FLAGS
	-outdir ${CMAKE_CURRENT_SOURCE_DIR}
	-I${CMAKE_CURRENT_SOURCE_DIR}/external -I${CMAKE_CURRENT_SOURCE_DIR}/../vendor
	${PROJECT_BINARY_DIR}/src/libhotspot.a
)
add_custom_target (matlab
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/external/obtainHotSpotModel.cpp ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/external/solveSSDTCWithHotSpot.cpp ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/external/solveSSTWithHotSpot.cpp ${MEX_FLAGS}
)
add_dependencies (matlab hotspot)
