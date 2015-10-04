cmake_minimum_required(VERSION 2.8)

project(hlsl-to-glsl)

set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set (SOURCE_FILES
	src/CodeGenerator.cpp
	src/HlslToGlsl.cpp
	src/main.cpp
	src/Tokenizer.cpp
)

set (HEADER_FILES
	include/CodeGenerator.h
	include/HlslToGlsl.h
	include/Tokenizer.h
)

source_group("Source Files" FILES ${SOURCE_FILES})
source_group("Header Files" FILES ${HEADER_FILES})

include_directories(include)

link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(
	hlsl-to-glsl
	${SOURCE_FILES}
	${HEADER_FILES}
)

target_link_libraries(
	hlsl-to-glsl
)