add_subdirectory (@Optima)
add_subdirectory (+Rainflow)
add_subdirectory (+Utils)

add_custom_target (matlab)
add_dependencies (matlab matlab-optima matlab-rainflow matlab-utils)
