set (MEX_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/matrix.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hotspot.cpp
)

foreach (version ${HOTSPOT_VERSIONS})
  set (MEX_LIBS
    ${VENDOR_DIR}/hotspot-${version}/libhotspot.a
  )

  set (MEX_FLAGS
    -outdir ${CMAKE_CURRENT_SOURCE_DIR}
    -I${CMAKE_CURRENT_SOURCE_DIR}
    -I${VENDOR_DIR}/numerical_recipes
    -DHOTSPOT_VERSION="${version}"
  )

  add_custom_target (mhotspot-${version}
    COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/obtain_coefficients.cpp
      ${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
    COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_original.cpp
      ${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
    COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_condensed_equation.cpp
      ${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
    COMMAND mex ${CMAKE_CURRENT_SOURCE_DIR}/solve_condensed_equation_with_leakage.cpp
      ${MEX_SRCS} ${MEX_LIBS} ${MEX_FLAGS}
  )

  add_custom_target (link-${version}
    COMMAND ln -sfn ${VENDOR_DIR}/hotspot-${version} ${CMAKE_CURRENT_SOURCE_DIR}/hotspot
  )

  add_dependencies (mhotspot-${version} hotspot-${version} link-${version})
endforeach (version)
