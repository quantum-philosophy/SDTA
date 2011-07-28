set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/+External)

# HotSpot library
add_definitions (-Wall -Wno-write-strings -O2 -mfpmath=sse -msse3 -pg -DDEBUG -fPIC)
set (hotspot_SRCS
	${PROJECT_SOURCE_DIR}/vendor/hotspot/flp.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/temperature.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/temperature_block.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/temperature_grid.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/RCutil.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/util.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/npe.c
	${PROJECT_SOURCE_DIR}/vendor/hotspot/shape.c
  ${EXTERNAL_DIR}/hotspot.c
)
include_directories (${PROJECT_SOURCE_DIR}/vendor)
set_source_files_properties (${hotspot_SRCS} PROPERTIES LANGUAGE CXX)
add_library (hotspot STATIC ${hotspot_SRCS})

# MatLab functions
set (MEX_FLAGS
  -outdir ${EXTERNAL_DIR}
  -I${EXTERNAL_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/../vendor
	${PROJECT_BINARY_DIR}/src/libhotspot.a
)
add_custom_target (matlab
  COMMAND mex ${EXTERNAL_DIR}/obtainHotSpotModel.cpp ${MEX_FLAGS}
	COMMAND mex ${EXTERNAL_DIR}/solveSSDTCWithHotSpot.cpp ${MEX_FLAGS}
	COMMAND mex ${EXTERNAL_DIR}/solveSSTWithHotSpot.cpp ${MEX_FLAGS}
)
add_dependencies (matlab hotspot)
