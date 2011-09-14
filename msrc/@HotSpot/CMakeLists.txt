set (MEX_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/mex_utils.cpp
	${PROJECT_SOURCE_DIR}/csrc/common.cpp
	${PROJECT_SOURCE_DIR}/csrc/utils.cpp
	${PROJECT_SOURCE_DIR}/csrc/Architecture.cpp
	${PROJECT_SOURCE_DIR}/csrc/Processor.cpp
	${PROJECT_SOURCE_DIR}/csrc/Task.cpp
	${PROJECT_SOURCE_DIR}/csrc/Hotspot.cpp
)

set (MEX_LIBS
	${VENDOR_DIR}/hotspot-${HOTSPOT_VERSION}/libhotspot.a
)

set (MEX_FLAGS
	-outdir ${CMAKE_CURRENT_SOURCE_DIR}
	-I${VENDOR_DIR}
	-I${PROJECT_SOURCE_DIR}/csrc
	-I${CMAKE_CURRENT_SOURCE_DIR}
	-I${VENDOR_DIR}/numerical_recipes
	-DHOTSPOT_VERSION="${HOTSPOT_VERSION}"
)

add_custom_target (matlab-hotspot
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/obtain_coefficients.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_condensed_equation.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_condensed_equation_with_leakage.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
)

add_dependencies (matlab-hotspot hotspot)
