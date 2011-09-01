set (EO_DIR ${VENDOR_DIR}/eo)
set (EO_LIBRARY_DIR ${EO_DIR}/release/lib)
set (EO_INCLUDE_DIR ${EO_DIR}/src)

set (COMMON_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Task.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Graph.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Processor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Architecture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/GeneticListScheduler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Hotspot.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ListScheduler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DynamicPower.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lifetime.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MD5Digest.cpp
)

set (EO_LIBS
	${EO_LIBRARY_DIR}/libeoutils.a
	${EO_LIBRARY_DIR}/libeo.a
	${VENDOR_DIR}/hotspot/libhotspot.a
)

set (EO_FLAGS
	-outdir ${CMAKE_CURRENT_SOURCE_DIR}
	-I${CMAKE_CURRENT_SOURCE_DIR}
	-I${EO_INCLUDE_DIR}
	-I${VENDOR_DIR}/numerical_recipes
	-I${VENDOR_DIR}
)

add_custom_target (genetics
	COMMAND mex
		${CMAKE_CURRENT_SOURCE_DIR}/optimizeAging.cpp
		${COMMON_SRCS} ${EO_LIBS} ${EO_FLAGS}
)

# A stand alone executable
add_executable (optima optima.cpp ${COMMON_SRCS})

add_dependencies (optima hotspot eo)

target_link_libraries (optima libeo libeoutils libhotspot)

include_directories (
	${EO_INCLUDE_DIR}
	${VENDOR_DIR}/numerical_recipes
	${VENDOR_DIR}
)

set(CMAKE_CXX_FLAGS "-O3")

add_library(libeo STATIC IMPORTED)
set_property(
	TARGET libeo PROPERTY
	IMPORTED_LOCATION ${EO_LIBRARY_DIR}/libeo.a
)

add_library(libeoutils STATIC IMPORTED)
set_property(
	TARGET libeoutils PROPERTY
	IMPORTED_LOCATION ${EO_LIBRARY_DIR}/libeoutils.a
)

add_library(libhotspot STATIC IMPORTED)
set_property(
	TARGET libhotspot PROPERTY
	IMPORTED_LOCATION ${VENDOR_DIR}/hotspot/libhotspot.a
)
