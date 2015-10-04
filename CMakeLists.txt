cmake_minimum_required (VERSION 2.8)

set (
    CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/CMakeModules"
)

if (WIN32)
else (WIN32)
    add_definitions(-std=c++11)
endif (WIN32)

add_subdirectory (hlsl-to-glsl)