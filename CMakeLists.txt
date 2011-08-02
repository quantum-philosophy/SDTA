cmake_minimum_required (VERSION 2.6)

project (ssdtc)

set (VENDOR_DIR ${PROJECT_SOURCE_DIR}/vendor)
set (HOTSPOT_VERSIONS 5.0 5.01)
set (HOTSPOT_DEFAULT_VERSION 5.01)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory (src)
add_subdirectory (vendor)
