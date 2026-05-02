cmake_minimum_required(VERSION 3.31)

## =======================================================================
## This Project is for implementing the way of rendering                 =
## =======================================================================

file(
	GLOB_RECURSE GPC_RENDER_SOURCES
	CONFIG_DEPENDS 
		"source/*.c"
		"source/*.cpp"
)
message("GPC_Render Sources : " ${GPC_RENDER_SOURCES})

file(
	GLOB_RECURSE GPC_RENDER_HEADERS
	CONFIG_DEPENDS 
		"include/*.h"
		"include/*.hpp"
		"include/*.inl"
)
message("GPC_Render Sources : " ${GPC_RENDER_SOURCES})

find_package(Freetype REQUIRED)
SET(CMAKE_EXE_LINKER_FLAGS "-v")

add_library(
	GPC_Render
		STATIC
		${GPC_RENDER_SOURCES}
	${GPC_RENDER_HEADERS}
)
target_compile_features(
	GPC_Render
	PUBLIC 
		cxx_std_20
)
target_include_directories(
	GPC_Render 
	PUBLIC 
		"include/"
		${FREETYPE_INCLUDE_DIRS}
)

if(NOT TARGET GPC_Framework)
	add_subdirectory(
			${CMAKE_CURRENT_SOURCE_DIR}/../GPC_Framework
			${CMAKE_CURRENT_BINARY_DIR}/GPC_Framework
	)
endif()

target_link_libraries(
	GPC_Render
	PUBLIC
		GPC_Framework
		${FREETYPE_LIBRARIES}
)