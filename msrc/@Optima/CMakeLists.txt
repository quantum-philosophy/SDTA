set (TINYMT_DIR ${VENDOR_DIR}/tinymt-1.0.1/tinymt)

set (MEX_SRCS
	${PROJECT_SOURCE_DIR}/csrc/AnalyticalSolution.cpp
	${PROJECT_SOURCE_DIR}/csrc/Architecture.cpp
	${PROJECT_SOURCE_DIR}/csrc/Constrain.cpp
	${PROJECT_SOURCE_DIR}/csrc/DynamicPower.cpp
	${PROJECT_SOURCE_DIR}/csrc/Graph.cpp
	${PROJECT_SOURCE_DIR}/csrc/GraphAnalysis.cpp
	${PROJECT_SOURCE_DIR}/csrc/Hotspot.cpp
	${PROJECT_SOURCE_DIR}/csrc/Layout.cpp
	${PROJECT_SOURCE_DIR}/csrc/Priority.cpp
	${PROJECT_SOURCE_DIR}/csrc/Processor.cpp
	${PROJECT_SOURCE_DIR}/csrc/Schedule.cpp
	${PROJECT_SOURCE_DIR}/csrc/Task.cpp
	${PROJECT_SOURCE_DIR}/csrc/Tuning.cpp
	${PROJECT_SOURCE_DIR}/csrc/common.cpp
	${PROJECT_SOURCE_DIR}/csrc/matrix.cpp
	${TINYMT_DIR}/tinymt64.c
)

set (MEX_LIBS
	${VENDOR_DIR}/hotspot-${HOTSPOT_VERSION}/libhotspot.a
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (MEX_FLAGS
		-outdir ${CMAKE_CURRENT_SOURCE_DIR}
		-I${PROJECT_SOURCE_DIR}/csrc
		-I${CMAKE_CURRENT_SOURCE_DIR}
		-I${VENDOR_DIR}
		-I${TINYMT_DIR}
		-DHOTSPOT_VERSION="${HOTSPOT_VERSION}"
		-DMEASURE_TIME
		-DSHALLOW_CHECK
		CFLAGS='-std=c99'
		LDFLAGS='$$LDFLAGS -lrt'
	)
else ()
	set (MEX_FLAGS
		-outdir ${CMAKE_CURRENT_SOURCE_DIR}
		-I${PROJECT_SOURCE_DIR}/csrc
		-I${CMAKE_CURRENT_SOURCE_DIR}
		-I${VENDOR_DIR}
		-I${TINYMT_DIR}
		-DHOTSPOT_VERSION="${HOTSPOT_VERSION}"
		-DMEASURE_TIME
		-DSHALLOW_CHECK
		CFLAGS='-std=c99'
	)
endif ()

add_custom_target (matlab-optima
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_coarse.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_power.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}

	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/verify.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/verify_power.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}

	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/get_coefficients.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/get_power.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}

	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/predict.cpp
		${PROJECT_SOURCE_DIR}/csrc/Lifetime.cpp ${MEX_FLAGS}
)
