set (PARADISEO_DIR ${VENDOR_DIR}/paradiseo)
set (TINYMT_DIR ${VENDOR_DIR}/tinymt-1.0.1/tinymt)

set (COMMON_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Task.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Graph.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Processor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Architecture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Pool.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Schedule.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Layout.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Priority.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Mobility.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Constrain.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/EvolutionTuning.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/EvolutionStats.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Evaluation.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SOEvolution.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MOEvolution.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Hotspot.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DynamicPower.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lifetime.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Random.cpp
	${TINYMT_DIR}/tinymt64.c
)

set_property(SOURCE ${TINYMT_DIR}/tinymt64.c
	APPEND PROPERTY COMPILE_FLAGS "-std=c99")

add_executable (optima optima.cpp ${COMMON_SRCS})

target_link_libraries (optima crypto memcached libeo libmoeo libeoutils libhotspot)

include_directories (
	${PARADISEO_DIR}/paradiseo-eo/src
	${PARADISEO_DIR}/paradiseo-mo/src
	${PARADISEO_DIR}/paradiseo-moeo/src
	${VENDOR_DIR}/numerical_recipes
	${VENDOR_DIR}
	${TINYMT_DIR}
)

# Available definitions:
#
# * REAL_RANK - ranking in doubles.
# * SHALLOW_CHECK - eliminate excessive checks.
# * UNCONSTRAINED - the genes do not have any constrains.
# * FAKE_EVALUATION - consider power instead of temperature.
# * WITHOUT_MEMCACHED - do not mention memcached in the source code.
# * VERIFY_CACHING - check what we are reading from memcached.
#
set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated-declarations -Wno-sign-compare -O3 $ENV{CFLAGS}")

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
