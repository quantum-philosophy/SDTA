set(HOTSPOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/+HotSpot)
set(RAINFLOW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/+Rainflow)

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
  ${HOTSPOT_DIR}/hotspot.c
)
include_directories (${PROJECT_SOURCE_DIR}/vendor)
set_source_files_properties (${hotspot_SRCS} PROPERTIES LANGUAGE CXX)
add_library (hotspot STATIC ${hotspot_SRCS})

# MatLab functions
set (MEX_HOTSPOT_FLAGS
  -outdir ${HOTSPOT_DIR}
  -I${HOTSPOT_DIR}
  -I${CMAKE_CURRENT_SOURCE_DIR}/../vendor
	${PROJECT_BINARY_DIR}/src/libhotspot.a
)
set (MEX_RAINFLOW_FLAGS
  -outdir ${RAINFLOW_DIR}
)
add_custom_target (matlab
  COMMAND mex ${HOTSPOT_DIR}/obtainModel.cpp ${MEX_HOTSPOT_FLAGS}
  COMMAND mex ${HOTSPOT_DIR}/solveSSDTC.cpp ${MEX_HOTSPOT_FLAGS}
  COMMAND mex ${HOTSPOT_DIR}/solveSST.cpp ${MEX_HOTSPOT_FLAGS}
  COMMAND mex ${RAINFLOW_DIR}/rainflow.c ${MEX_RAINFLOW_FLAGS}
)
add_dependencies (matlab hotspot)
