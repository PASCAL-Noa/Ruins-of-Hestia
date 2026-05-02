cmake_minimum_required(VERSION 3.31)

## =======================================================================
## This Project is for the ECS and linking it to the render and a Window =
## =======================================================================

file(
	GLOB_RECURSE GPC_ENGINE_SOURCES
	CONFIGURE_DEPENDS 
		"source/*.c"
		"source/*.cpp"
)
message("GPC_Engine Sources : " ${GPC_ENGINE_SOURCES})

file(
	GLOB_RECURSE GPC_ENGINE_HEADERS
	CONFIGURE_DEPENDS 
		"include/*.h"
		"include/*.hpp"
		"include/*.inl"
)
message("GPC_Engine Headers : " ${GPC_ENGINE_HEADERS})

add_library(
	GPC_Engine
		STATIC
		${GPC_ENGINE_SOURCES}
	${GPC_ENGINE_HEADERS}
)
target_compile_features(
	GPC_Engine 
	PUBLIC 
		cxx_std_20
)
target_include_directories(
	GPC_Engine
	PUBLIC
		"include/"
)

target_precompile_headers(GPC_Engine PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/EnginePCH.h>")

if(NOT TARGET GPC_Assets)
	add_subdirectory(
			${CMAKE_CURRENT_SOURCE_DIR}/../GPC_Assets
			${CMAKE_CURRENT_BINARY_DIR}/GPC_Assets
	)
endif()

target_link_libraries(
	GPC_Engine
	PUBLIC
		GPC_Audio
		GPC_Render
		GPC_Assets
)