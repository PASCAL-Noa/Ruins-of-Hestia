cmake_minimum_required(VERSION 3.31)

## =======================================================================
## This Project is for defining every basis of our Engine                =
## =======================================================================


file(
	GLOB_RECURSE GPC_FRAMEWORK_SOURCES
	CONFIG_DEPENDS 
		"source/*.c"
		"source/*.cpp"
)
message("GPC_Framework Sources : " ${GPC_FRAMEWORK_SOURCES})


file(
	GLOB_RECURSE GPC_FRAMEWORK_HEADERS
	CONFIG_DEPENDS 
		"include/*.h"
		"include/*.hpp"
		"include/*.inl"
)
message("GPC_Framework Sources : " ${GPC_FRAMEWORK_HEADERS})


add_library(
	GPC_Framework
		STATIC
		${GPC_FRAMEWORK_SOURCES}
	${GPC_FRAMEWORK_HEADERS}
        include/Stream.h
        source/Stream.cpp
        include/Blob.h
		source/Blob.cpp
		include/File.h
		source/File.cpp
        include/ArchiveManager.h
		source/ArchiveManager.cpp
)
target_compile_features(
	GPC_Framework
	PUBLIC 
		cxx_std_20
)
target_include_directories(
	GPC_Framework
	PUBLIC
		"include/"
)

target_precompile_headers(GPC_Framework PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/FrameworkPCH.h>")

## =======================================================================
## Configuration Multi-plateforme RtAudio			                     =
## =======================================================================

if(WIN32)
	# Windows : WASAPI
	target_compile_definitions(GPC_Framework PUBLIC __WINDOWS_WASAPI__)
	target_link_libraries(GPC_Framework PRIVATE
			ole32 setupapi ksuser winmm uuid mfplat mfuuid wmcodecdspuuid
	)
elseif(APPLE)
	# macOS : CoreAudio
	target_compile_definitions(GPC_Framework PUBLIC __MACOSX_CORE__)
	target_link_libraries(GPC_Framework PRIVATE
			"-framework CoreAudio"
			"-framework CoreFoundation"
			"-framework AudioToolbox"
			"-framework AudioUnit"
	)
else()
	# Linux : ALSA
	target_compile_definitions(GPC_Framework PUBLIC __LINUX_ALSA__)
	target_link_libraries(GPC_Framework PRIVATE asound pthread)
endif()

include(find_fbxsdk.cmake)

if (FBXSDK_FOUND)
	target_include_directories( GPC_Framework PUBLIC FBXSDK_INCLUDE_DIR)
	target_link_libraries( GPC_Framework
			PUBLIC
			${FBXSDK_LIBRARY})
endif()

set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)

find_package(glfw3 CONFIG REQUIRED)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
get_property(importedTargets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)
message("Imported targets: ${importedTargets}")

target_link_libraries(GPC_Framework
		PUBLIC
		Vulkan::Vulkan
		glfw
		glm::glm
)