set (MEX_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/matrix.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hotspot.cpp
)

set (MEX_LIBS
	${VENDOR_DIR}/hotspot-${HOTSPOT_VERSION}/libhotspot.a
)

set (MEX_FLAGS
	-outdir ${CMAKE_CURRENT_SOURCE_DIR}
	-I${CMAKE_CURRENT_SOURCE_DIR}
	-I${VENDOR_DIR}/numerical_recipes
	-DHOTSPOT_VERSION="${HOTSPOT_VERSION}"
)

add_custom_target (matlab-hotspot
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/obtain_coefficients.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_original.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_condensed_equation.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_condensed_equation_with_leakage.cpp
		${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
)

add_custom_target (hotspot-link
	COMMAND ln -sfn ${VENDOR_DIR}/hotspot-${HOTSPOT_VERSION} ${CMAKE_CURRENT_SOURCE_DIR}/hotspot
)

add_dependencies (matlab-hotspot hotspot hotspot-link)
