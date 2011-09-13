set (PARADISEO_DIR ${VENDOR_DIR}/paradiseo)

set (COMMON_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Task.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Graph.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Processor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Architecture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/GeneticListScheduler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SingleObjectiveGLS.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MultiObjectiveGLS.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Hotspot.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ListScheduler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DynamicPower.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lifetime.cpp
)

add_executable (optima optima.cpp ${COMMON_SRCS})

target_link_libraries (optima libeo libmoeo libeoutils libhotspot)

include_directories (
	${PARADISEO_DIR}/paradiseo-eo/src
	${PARADISEO_DIR}/paradiseo-mo/src
	${PARADISEO_DIR}/paradiseo-moeo/src
	${VENDOR_DIR}/numerical_recipes
	${VENDOR_DIR}
)

set(CMAKE_CXX_FLAGS "-DINT_RANK -O3")

add_library(libeo STATIC IMPORTED)
set_property(
	TARGET libeo PROPERTY
	IMPORTED_LOCATION ${PARADISEO_DIR}/paradiseo-eo/build/lib/libeo.a
)

add_library(libmoeo STATIC IMPORTED)
set_property(
	TARGET libmoeo PROPERTY
	IMPORTED_LOCATION ${PARADISEO_DIR}/paradiseo-moeo/build/lib/libmoeo.a
)

add_library(libeoutils STATIC IMPORTED)
set_property(
	TARGET libeoutils PROPERTY
	IMPORTED_LOCATION ${PARADISEO_DIR}/paradiseo-eo/build/lib/libeoutils.a
)

add_library(libhotspot STATIC IMPORTED)
set_property(
	TARGET libhotspot PROPERTY
	IMPORTED_LOCATION ${VENDOR_DIR}/hotspot/libhotspot.a
)

# Tools
add_executable (floorplan floorplan.cpp)
add_executable (system system.cpp)
