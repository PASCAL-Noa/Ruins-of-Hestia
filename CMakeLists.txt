cmake_minimum_required(VERSION 3.31)

project(Ruins_of_Hestia)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")

find_program(CCACHE_PROGRAM ccache)

if(NOT CCACHE_PROGRAM AND WIN32)
	set(VENDORED_CCACHE "${CMAKE_SOURCE_DIR}/tools/ccache/ccache.exe")
	if(EXISTS "${VENDORED_CCACHE}")
		set(CCACHE_PROGRAM "${VENDORED_CCACHE}")
	endif()
endif()

if(CCACHE_PROGRAM)
	set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
	message(STATUS "ccache active: ${CCACHE_PROGRAM}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")


# assume built-in pthreads on MacOS
IF(UNIX)

	set(CMAKE_THREAD_LIBS_INIT "-lpthread")
	set(CMAKE_HAVE_THREADS_LIBRARY 1)
	set(CMAKE_USE_WIN32_THREADS_INIT 0)
	set(CMAKE_USE_PTHREADS_INIT 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	add_definitions( -DGPC_LINUX )
ELSE ()
	set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static -mwindows")
ENDIF()

if(NOT TARGET GPC_Framework)
	add_subdirectory("Libraries/GPC_Framework")
endif()

if(NOT TARGET GPC_Render)
	add_subdirectory("Libraries/GPC_Render")
endif()

if(NOT TARGET GPC_Audio)
	add_subdirectory("Libraries/GPC_Audio")
endif()

if(NOT TARGET GPC_Assets)
	add_subdirectory("Libraries/GPC_Assets")
endif()

if(NOT TARGET GPC_Engine)
	add_subdirectory("Libraries/GPC_Engine")
endif()

add_subdirectory("Game")