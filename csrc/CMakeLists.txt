# Available tuning flags:
#
# * REAL_RANK - ranking in doubles.
# * SHALLOW_CHECK - eliminate excessive checks.
# * UNCONSTRAINED - the genes do not have any constrains.
# * WITHOUT_MEMCACHED - do not mention memcached in the source code.
# * VERIFY_CACHING - check what we are reading from memcached.
# * MEASURE_TIME - measure time.
# * EXTENDED_STATS - diversity.
# * PRECISE_TIMEOUT - precise timeout measurement, after each evaluation.
#
set(TUNING_FLAGS "-DWITHOUT_MEMCACHED -DSHALLOW_CHECK")

set (PARADISEO_DIR ${VENDOR_DIR}/paradiseo)
set (TINYMT_DIR ${VENDOR_DIR}/tinymt-1.0.1/tinymt)

set (OPTIMA_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/AnalyticalSolution.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Architecture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Constrain.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DynamicPower.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Evaluation.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/EvolutionStats.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Graph.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/GraphAnalysis.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Hotspot.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Layout.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lifetime.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MOEvolution.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Priority.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Processor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Random.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SOEvolution.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Schedule.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Task.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Tuning.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/matrix.cpp
	${TINYMT_DIR}/tinymt64.c
)

set (SOLVE_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/AnalyticalSolution.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Architecture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DynamicPower.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Graph.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/GraphAnalysis.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Hotspot.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Layout.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Priority.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Processor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Schedule.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Task.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Tuning.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/matrix.cpp
)

set_property(SOURCE ${TINYMT_DIR}/tinymt64.c
	APPEND PROPERTY COMPILE_FLAGS "-std=c99")

add_executable (optima optima.cpp ${OPTIMA_SRCS})

if (${TUNING_FLAGS} MATCHES "WITHOUT_MEMCACHED")
	set (MEMCACHED_LIBS "")
else ()
	set (MEMCACHED_LIBS crypto memcached)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (optima
		${MEMCACHED_LIBS} libeo libmoeo libeoutils libhotspot rt)
else ()
	target_link_libraries (optima
		${MEMCACHED_LIBS} libeo libmoeo libeoutils libhotspot)
endif ()

add_executable (solve solve.cpp ${SOLVE_SRCS})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (solve libhotspot rt)
else ()
	target_link_libraries (solve libhotspot)
endif ()

include_directories (
	${PARADISEO_DIR}/paradiseo-eo/src
	${PARADISEO_DIR}/paradiseo-mo/src
	${PARADISEO_DIR}/paradiseo-moeo/src
	${VENDOR_DIR}
	${TINYMT_DIR}
)

set(CMAKE_CXX_FLAGS "${TUNING_FLAGS} -Wall -Wno-deprecated-declarations -Wno-sign-compare -O3 $ENV{CFLAGS}")

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
