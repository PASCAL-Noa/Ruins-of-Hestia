#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

#include "Debugger.h"

#define GPC_FAILED(code) (code != GPC::ErrorType::SUCCESS)

#ifndef NDEBUG
#define GPC_SUCCESS(code) {GPC::ErrorType ___GPC_SUCCESS___ = code; \
if (GPC_FAILED(___GPC_SUCCESS___))                                  \
    { GPC_ERROR << GPC::ErrorMap[___GPC_SUCCESS___] << ENDL; }}
#else
#define GPC_SUCCESS(code) code;
#endif

#ifndef NDEBUG
#define GPC_ASSERT(condition) if(!(condition))
#define GPC_DEBUG_ONLY if(true)
#else
#define GPC_ASSERT(condition) if(false)
#define GPC_DEBUG_ONLY if(false)
#endif

namespace GPC
{

    enum class ErrorType : uint32_t
    {
        SUCCESS,

        //// WINDOW ERRORS ////
        WINDOW_INITIALIZATION_FAILED,
        WINDOW_SURFACE_CREATION_FAILED,
        WINDOW_SURFACE_ACQUIREMENT,

        //// VULKAN ERROR ////
        VULKAN_ALREADY_ALLOCATED,
        VULKAN_DEVICE_INSTANCE_CREATION,
        VULKAN_DEVICE_CREATION_FAILED,
        VULKAN_DEVICE_VALIDATION_LAYER,
        VULKAN_DEVICE_DEBUG_CREATION,
        VULKAN_DEVICE_NO_SUITABLE_GPU,
        VULKAN_CONTEXT_COMMAND_BUFFER,
        VULKAN_CONTEXT_COMMAND_POOL,
        VULKAN_SHADER_INVALID_FILE,
        VULKAN_BUFFER_ALLOCATION,
        VULKAN_BUFFER_CREATION,
        VULKAN_TEXTURE_LOAD,
        VULKAN_TEXTURE_CREATION,
        VULKAN_TEXTURE_VIEW_CREATION,
        VULKAN_TEXTURE_TRANSITION,
        VULKAN_RENDER_TARGET_CREATION,
        VULKAN_RENDER_FRAMEBUFFER_CREATION,
        VULKAN_RENDER_SWAP_CHAIN_CREATION,
        VULKAN_RENDER_SYNCHRONIZATION_CREATION,
        VULKAN_PROGRAM_BINDINGS_CREATION,
        VULKAN_PROGRAM_PIPELINE_CREATION,
        VULKAN_PROGRAM_POOL_CREATION,
        VULKAN_PROGRAM_ALLOCATION_CREATION,
        VULKAN_SAMPLER_CREATION,

        //// Font loading ////
        FT_FONT_LIB_INITIALIZATION,
        FT_FONT_NOT_LOAD,
        FT_FONT_CHAR_NOT_FOUND,

        //// ECS ////
        SYSTEM_CREATION_FAILED,

        //// Serializer Errors ////
        FILE_NOT_FOUND,
        FILE_WRITE_FAILED,
        FILE_READ_FAILED,
        INVALID_MAGIC,
        INVALID_VERSION,
        INVALID_CHECKSUM,
        INVALID_KEY_PATH,
        TREE_NOT_MAP,
        ALLOC_FAILED,

        END
    };

    inline static std::unordered_map<ErrorType, std::string_view > ErrorMap {

        { ErrorType::SUCCESS, "Success" },

        //// WINDOW ERRORS ////
        { ErrorType::WINDOW_INITIALIZATION_FAILED, "Window Initialization Failed" },
        { ErrorType::WINDOW_SURFACE_CREATION_FAILED, "Window Surface Creation Failed" },
        { ErrorType::WINDOW_SURFACE_ACQUIREMENT, "Window Surface Acquirement Failed" },

        //// VULKAN ERROR ////
        { ErrorType::VULKAN_ALREADY_ALLOCATED, "Vulkan Already Allocated" },
        { ErrorType::VULKAN_DEVICE_INSTANCE_CREATION, "Vulkan Device Instance Creation Failed" },
        { ErrorType::VULKAN_DEVICE_CREATION_FAILED, "Vulkan Device Creation Failed" },
        { ErrorType::VULKAN_DEVICE_VALIDATION_LAYER, "Vulkan Device Validation Layer Error" },
        { ErrorType::VULKAN_DEVICE_DEBUG_CREATION, "Vulkan Device Debug Creation Failed" },
        { ErrorType::VULKAN_DEVICE_NO_SUITABLE_GPU, "Vulkan No Suitable GPU Found" },
        { ErrorType::VULKAN_CONTEXT_COMMAND_BUFFER, "Vulkan Command Buffer Creation Failed" },
        { ErrorType::VULKAN_CONTEXT_COMMAND_POOL, "Vulkan Command Pool Creation Failed" },
        { ErrorType::VULKAN_SHADER_INVALID_FILE, "Vulkan Shader Invalid File" },
        { ErrorType::VULKAN_BUFFER_ALLOCATION, "Vulkan Buffer Allocation Failed" },
        { ErrorType::VULKAN_BUFFER_CREATION, "Vulkan Buffer Creation Failed" },
        { ErrorType::VULKAN_TEXTURE_LOAD, "Vulkan Texture Load Failed" },
        { ErrorType::VULKAN_TEXTURE_CREATION, "Vulkan Texture Creation Failed" },
        { ErrorType::VULKAN_TEXTURE_VIEW_CREATION, "Vulkan Texture View Creation Failed" },
        { ErrorType::VULKAN_TEXTURE_TRANSITION, "Vulkan Texture Transition Failed" },
        { ErrorType::VULKAN_RENDER_TARGET_CREATION, "Vulkan Render Target Creation Failed" },
        { ErrorType::VULKAN_RENDER_FRAMEBUFFER_CREATION, "Vulkan Render Framebuffer Creation Failed" },
        { ErrorType::VULKAN_RENDER_SWAP_CHAIN_CREATION, "Vulkan Swap Chain Creation Failed" },
        { ErrorType::VULKAN_RENDER_SYNCHRONIZATION_CREATION, "Vulkan Render Synchronization Creation Failed" },
        { ErrorType::VULKAN_PROGRAM_BINDINGS_CREATION, "Vulkan Program Bindings Creation Failed" },
        { ErrorType::VULKAN_PROGRAM_PIPELINE_CREATION, "Vulkan Program Pipeline Creation Failed" },
        { ErrorType::VULKAN_PROGRAM_POOL_CREATION, "Vulkan Program Pool Creation Failed" },
        { ErrorType::VULKAN_PROGRAM_ALLOCATION_CREATION, "Vulkan Program Allocation Creation Failed" },
        { ErrorType::VULKAN_SAMPLER_CREATION, "Vulkan Sampler Creation Failed" },

        //// ECS ////
        { ErrorType::SYSTEM_CREATION_FAILED, "System Creation Failed" },

        { ErrorType::END, "Unknown Error" }

    };

}
