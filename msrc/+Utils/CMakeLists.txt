set (MEX_FLAGS
	-outdir ${CMAKE_CURRENT_SOURCE_DIR}
	-I${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (matlab-utils
	COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/mMD5.c ${MEX_FLAGS}
)
