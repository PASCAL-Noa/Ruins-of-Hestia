#include "Shader.h"

#include <stdexcept>

#include "RenderDevice.h"

namespace GPC
{

    Shader::Shader() = default;
    Shader::~Shader() = default;

    ErrorType Shader::Create(std::string shaderPath, Type shaderType, const char *entryPoint)
    {
        std::string fullPath = "Resources/Shaders/" + shaderPath + ".spv";
        std::vector<char> shaderCode;
        try {
            shaderCode = ReadFile(fullPath);
        } catch (const std::runtime_error&) {
            return ErrorType::VULKAN_SHADER_INVALID_FILE;
        }

        if (shaderCode.empty()) {
            return ErrorType::VULKAN_SHADER_INVALID_FILE;
        }

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = VK_NULL_HANDLE;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        if (vkCreateShaderModule(RenderDevice::GetDevice(), &createInfo, nullptr, &m_shaderModule) != VK_SUCCESS) {
            return ErrorType::VULKAN_SHADER_INVALID_FILE;
        }

        m_pipelineShaderStageInfo = VkPipelineShaderStageCreateInfo();
        m_pipelineShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        m_pipelineShaderStageInfo.stage = static_cast<VkShaderStageFlagBits>(shaderType);
        m_pipelineShaderStageInfo.module = m_shaderModule;
        m_pipelineShaderStageInfo.pName = entryPoint;

        return ErrorType::SUCCESS;
    }

    void Shader::Destroy()
    {
        vkDestroyShaderModule(RenderDevice::GetDevice(), m_shaderModule, nullptr);
    }

    std::vector<char> Shader::ReadFile(const std::string& filename) // TODO REPLACE WITH A FILE SYSTEM IF WE HAVE
    {

        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open " + filename );
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    VkPipelineShaderStageCreateInfo const& Shader::GetShaderInformation()
    {
        return m_pipelineShaderStageInfo;
    }

} // GPC