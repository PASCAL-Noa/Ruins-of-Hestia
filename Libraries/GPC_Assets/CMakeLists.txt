cmake_minimum_required(VERSION 3.31)

## =======================================================================
## This Project is for implementing the way of audio                 =
## =======================================================================

file(
	GLOB_RECURSE GPC_ASSETS_SOURCES
	CONFIG_DEPENDS 
		"source/*.c"
		"source/*.cpp"
)
message("GPC_Render Sources : " ${GPC_ASSETS_SOURCES})

file(
	GLOB_RECURSE GPC_ASSETS_HEADERS
	CONFIG_DEPENDS 
		"include/*.h"
		"include/*.hpp"
		"include/*.inl"
)
message("GPC_Render Sources : " ${GPC_ASSETS_SOURCES})

add_library(
	GPC_Assets
		STATIC
		${GPC_ASSETS_SOURCES}
	${GPC_ASSETS_HEADERS}
		include/Loader.h
		source/Loader.cpp
		source/Meta/MetaTypes.cpp
		include/Meta/MetaTypes.h
		source/Meta/MetaAsset.cpp
		include/Meta/MetaAsset.h
		source/Meta/MetaFile.cpp
		include/Meta/MetaFile.h
		source/Meta/MetaReader.cpp
		include/Meta/MetaReader.h
		source/Meta/MetaWriter.cpp
		include/Meta/MetaWriter.h
		source/Meta/MetaLoader.cpp
		include/Meta/MetaLoader.h
		source/Meta/MetaBuilder.cpp
		include/Meta/MetaBuilder.h
)
target_compile_features(
	GPC_Assets
	PUBLIC
		cxx_std_20
)
target_include_directories(
	GPC_Assets
	PUBLIC
		"include/"
)

if(NOT TARGET GPC_Render)
	add_subdirectory(
			${CMAKE_CURRENT_SOURCE_DIR}/../GPC_Render
			${CMAKE_CURRENT_BINARY_DIR}/GPC_Render
	)
endif()

if(NOT TARGET GPC_Audio)
	add_subdirectory(
			${CMAKE_CURRENT_SOURCE_DIR}/../GPC_Audio
			${CMAKE_CURRENT_BINARY_DIR}/GPC_Audio
	)
endif()

target_link_libraries(
	GPC_Assets
	PUBLIC
		GPC_Audio
		GPC_Render
)