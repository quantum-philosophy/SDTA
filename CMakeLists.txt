cmake_minimum_required (VERSION 2.6)

project (ssdtc)

set (VENDOR_DIR ${PROJECT_SOURCE_DIR}/vendor)
set (HOTSPOT_VERSIONS 5.0 5.01)
set (HOTSPOT_DEFAULT_VERSION 5.01)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory (src)
add_subdirectory (vendor)

add_custom_target (cleanup
  COMMAND rm -f ${PROJECT_BINARY_DIR}/*.vcg
  COMMAND rm -f ${PROJECT_BINARY_DIR}/*.eps
  COMMAND rm -f ${PROJECT_BINARY_DIR}/*.flp
  COMMAND rm -f ${PROJECT_BINARY_DIR}/*.tgff
  COMMAND rm -f ${PROJECT_BINARY_DIR}/*.ptrace*
  COMMAND rm -f ${PROJECT_BINARY_DIR}/*.ttrace*
)
