cmake_minimum_required(VERSION 3.31)

## =======================================================================
## This Project is for implementing the way of audio                 =
## =======================================================================

file(
	GLOB_RECURSE GPC_AUDIO_SOURCES
	CONFIG_DEPENDS 
		"source/*.c"
		"source/*.cpp"
)
message("GPC_Render Sources : " ${GPC_AUDIO_SOURCES})

file(
	GLOB_RECURSE GPC_AUDIO_HEADERS
	CONFIG_DEPENDS 
		"include/*.h"
		"include/*.hpp"
		"include/*.inl"
)
message("GPC_Render Sources : " ${GPC_AUDIO_SOURCES})

add_library(
	GPC_Audio
		STATIC
		${GPC_AUDIO_SOURCES}
	${GPC_AUDIO_HEADERS}
)
target_compile_features(
	GPC_Audio
	PUBLIC
		cxx_std_20
)
target_include_directories(
	GPC_Audio
	PUBLIC
		"include/"
)

if(NOT TARGET GPC_Framework)
	add_subdirectory(
			${CMAKE_CURRENT_SOURCE_DIR}/../GPC_Framework
			${CMAKE_CURRENT_BINARY_DIR}/GPC_Framework
	)
endif()

target_link_libraries(
	GPC_Audio
	PUBLIC
		GPC_Framework
)