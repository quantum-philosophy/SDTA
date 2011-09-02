set (MEX_FLAGS
	-outdir ${CMAKE_CURRENT_SOURCE_DIR}
	-I${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (matlab-rainflow
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/rainflow.c ${MEX_FLAGS}
)
