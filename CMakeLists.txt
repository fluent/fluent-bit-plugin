cmake_minimum_required(VERSION 2.8)
project(fluent-bit-plugin)

# Macro to build source code
macro(FLB_PLUGIN name src deps)
  add_library(flb-${name} SHARED ${src})
  set_target_properties(flb-${name} PROPERTIES PREFIX "")
  set_target_properties(flb-${name} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  target_link_libraries(flb-${name} ${deps})
endmacro()

# Fluent Bit source code environment
if(NOT DEFINED FLB_SOURCE OR FLB_SOURCE STREQUAL "")
  message(FATAL_ERROR "No Fluent Bit source directory has been defined")
endif()

# Validate plugin name
if(NOT DEFINED PLUGIN_NAME OR PLUGIN_NAME STREQUAL "")
  message(FATAL_ERROR "Plugin name is not defined")
endif()

# Info
message(STATUS "Fluent Bit source code: " ${FLB_SOURCE})

include_directories(${FLB_SOURCE}/include)
include_directories(${FLB_SOURCE}/lib/monkey/include/)
include_directories(${FLB_SOURCE}/lib/mbedtls-2.16.1/include/)
include_directories(${FLB_SOURCE}/lib/flb_libco/)
include_directories(${FLB_SOURCE}/lib/msgpack-3.2.0/include/)
include_directories(${FLB_SOURCE}/lib/onigmo)
include_directories(${FLB_SOURCE}/lib/)

# Build plugin
add_subdirectory(${PLUGIN_NAME})
