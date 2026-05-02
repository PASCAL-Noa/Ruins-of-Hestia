cmake_minimum_required(VERSION 3.31)

## =======================================================================
## Final game application build
## =======================================================================

file(
        GLOB_RECURSE RUINS_OF_HESTIA_SOURCES
        CONFIGURE_DEPENDS
        "source/*.c"
        "source/*.cpp"
)
message("Game Sources : " ${RUINS_OF_HESTIA_SOURCES})

file(
        GLOB_RECURSE RUINS_OF_HESTIA_HEADERS
        CONFIGURE_DEPENDS
        "include/*.h"
        "include/*.hpp"
        "include/*.inl"
)
message("Game Headers : " ${RUINS_OF_HESTIA_HEADERS})

add_executable(
        RUINS_OF_HESTIA
        ${RUINS_OF_HESTIA_SOURCES}
        ${RUINS_OF_HESTIA_HEADERS}
        include/RandomAudio.h
)

target_compile_features(
        RUINS_OF_HESTIA
        PUBLIC
        cxx_std_20
)
target_include_directories(
        RUINS_OF_HESTIA
        PUBLIC
        "include/"
)

if(NOT TARGET GPC_Engine)
    add_subdirectory(
            ${CMAKE_CURRENT_SOURCE_DIR}/../Libraries/GPC_Engine
            ${CMAKE_CURRENT_BINARY_DIR}/GPC_Engine
    )
endif()


target_link_libraries(
        RUINS_OF_HESTIA
        PUBLIC
            GPC_Engine
)

target_precompile_headers(RUINS_OF_HESTIA PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/GamePCH.h>")

set(SHADER_DIR ${PROJECT_SOURCE_DIR}/Resources/Shaders)

if ( WIN32 )
    add_custom_target(compile_shaders
            COMMAND ${SHADER_DIR}/compile.bat
            WORKING_DIRECTORY ${SHADER_DIR}
            COMMENT "Compiling GLSL shaders")

    add_dependencies(RUINS_OF_HESTIA compile_shaders)
endif()

# Fonction simple pour générer une cible de copie automatique
function(add_auto_copy NAME SOURCE_DIR DEST_DIR)
    file(GLOB_RECURSE FILES_TO_WATCH "${SOURCE_DIR}/*")

    add_custom_target(${NAME}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR} ${DEST_DIR}
            DEPENDS ${FILES_TO_WATCH}
            COMMENT "Copying: ${SOURCE_DIR} -> ${DEST_DIR}"
    )

    add_dependencies(RUINS_OF_HESTIA ${NAME})

    message(STATUS "Resource folder copied in project")
endfunction()

add_auto_copy(copy_shaders "${PROJECT_SOURCE_DIR}/Resources" "${CMAKE_BINARY_DIR}/bin/Resources")

add_dependencies(RUINS_OF_HESTIA copy_shaders)