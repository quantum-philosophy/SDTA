add_subdirectory (@HotSpot)
add_subdirectory (+Rainflow)
add_subdirectory (+Utils)

add_custom_target (matlab)
add_dependencies (matlab mhotspot-${HOTSPOT_DEFAULT_VERSION} mrainflow mutils)
