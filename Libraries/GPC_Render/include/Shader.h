#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "ErrorType.h"

namespace GPC
{
    class RenderDevice;

    struct ShaderStage
    {
        uint32_t            Binding;
        VkShaderStageFlags  Frags;

        VkDescriptorType    DescriptorType;
        uint32_t            DescriptorCount;
    };

    GPC_INHERIT_OBJECT(Shader)
    {

    public:

        typedef enum Type
        {
            VERTEX = 0x00000001,
            TESSELLATION_CONTROL = 0x00000002,
            TESSELLATION_EVALUATION = 0x00000004,
            GEOMETRY = 0x00000008,
            FRAGMENT = 0x00000010,
            COMPUTE = 0x00000020,
        } Type;

        Shader();
        ~Shader() override;

        ErrorType Create(std::string shaderPath, Type shaderType, const char* entryPoint = "main");
        void Destroy();

        VkPipelineShaderStageCreateInfo const& GetShaderInformation();

    private:
        static std::vector<char> ReadFile(const std::string& filename);

        std::vector<ShaderStage> m_stages;

        VkShaderModule m_shaderModule;
        VkPipelineShaderStageCreateInfo m_pipelineShaderStageInfo;

    };

} // GPC