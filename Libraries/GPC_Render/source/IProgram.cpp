#include "IProgram.h"

#include "Buffer.h"

namespace GPC
{

    DescriptorSetInformation DescriptorSetInformation::ForCamera(uint32_t binding)
    {
        return {
            .Name = "camera",
            .Type = DescriptorType::UNIFORM_BUFFER,
            .Usages = UNIFORM_BUFFER_BIT,
            .BindingSlot = binding,
            .DescriptorCount = 1,
            .ElementCount = 1,
            .ElementStride = sizeof(UniformCameraBuffer),
            .Flags = VK_SHADER_STAGE_VERTEX_BIT,
            .Sampler_ = nullptr
        };
    }

    DescriptorSetInformation DescriptorSetInformation::ForLights(uint32_t binding)
    {
        return {
            .Name = "lights",
            .Type = DescriptorType::UNIFORM_BUFFER,
            .Usages = UNIFORM_BUFFER_BIT,
            .BindingSlot = binding,
            .DescriptorCount = 1,
            .ElementCount = 1,
            .ElementStride = sizeof(UniformLightBuffer),
            .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .Sampler_ = nullptr
        };
    }

    DescriptorSetInformation DescriptorSetInformation::ForTransform(uint32_t binding)
    {
        return {
            .Name = "transforms",
            .Type = DescriptorType::STORAGE_BUFFER,
            .Usages = STORAGE_BUFFER_BIT,
            .BindingSlot = binding,
            .DescriptorCount = 1,
            .ElementCount = 1,
            .ElementStride = sizeof(TransformData),
            .Flags = VK_SHADER_STAGE_VERTEX_BIT,
            .Sampler_ = nullptr
        };
    }

    DescriptorSetInformation DescriptorSetInformation::ForMaterials(uint32_t binding)
    {
        return {
            .Name = "materials",
            .Type = DescriptorType::STORAGE_BUFFER,
            .Usages = STORAGE_BUFFER_BIT,
            .BindingSlot = binding,
            .DescriptorCount = 1,
            .ElementCount = 1,
            .ElementStride = sizeof(MaterialData),
            .Flags = VK_SHADER_STAGE_VERTEX_BIT,
            .Sampler_ = nullptr
        };
    }

    DescriptorSetInformation DescriptorSetInformation::ForImages(uint32_t binding, std::string const& id, uint32_t descriptorCount)
    {

        return {
            .Name = id,
            .Type = DescriptorType::COMBINED_IMAGE_SAMPLER,
            .BindingSlot = binding,
            .DescriptorCount = descriptorCount,
            .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .Sampler_ = nullptr
        };
    }

    DescriptorSetInformation DescriptorSetInformation::ForTextures(uint32_t binding)
    {
        return ForImages(binding, "images", 50);
    }

    ErrorType IProgram::CreateDescriptorLayout()
    {

        std::vector<VkDescriptorSetLayoutBinding> bindings(m_DescriptorSetInfo.size());
        for (uint32_t i = 0; i < m_DescriptorSetInfo.size(); i++) {
            DescriptorSetInformation& info = m_DescriptorSetInfo[i];
            bindings[i].binding             = info.BindingSlot;
            bindings[i].descriptorCount     = info.DescriptorCount;
            bindings[i].descriptorType      = static_cast<VkDescriptorType>(info.Type);
            bindings[i].stageFlags          = info.Flags;
            if (info.Sampler_ != nullptr)
                bindings[i].pImmutableSamplers  = &info.Sampler_->GetSampler();
            else
                bindings[i].pImmutableSamplers  = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings    = bindings.data();

        if (vkCreateDescriptorSetLayout(RenderDevice::GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
            return ErrorType::VULKAN_PROGRAM_BINDINGS_CREATION;
        }

        return ErrorType::SUCCESS;

    }

    void IProgram::CreateVertex(VertexInformation &vertexInformation) const
    {

        vertexInformation.InputCreateInfo.vertexAttributeDescriptionCount   = static_cast<uint32_t>(m_AttributeDescriptions.size());
        vertexInformation.InputCreateInfo.pVertexAttributeDescriptions      = m_AttributeDescriptions.data();
        vertexInformation.InputCreateInfo.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInformation.InputCreateInfo.vertexBindingDescriptionCount     = m_BindingsDescription.size();
        vertexInformation.InputCreateInfo.pVertexBindingDescriptions        = m_BindingsDescription.data();

        vertexInformation.AssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        vertexInformation.AssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        vertexInformation.AssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
    }

    void IProgram::CreateViewport(ViewportInformation &viewportInformation) const
    {
        viewportInformation.DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        viewportInformation.DynamicState.dynamicStateCount = static_cast<uint32_t>(m_DynamicStates.size());
        viewportInformation.DynamicState.pDynamicStates = m_DynamicStates.data();

        viewportInformation.ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInformation.ViewportState.viewportCount = 1;
        viewportInformation.ViewportState.scissorCount = 1;
    }

    void IProgram::CreateAttachments(ProgramInformation const &programInformation,
        AttachmentsInformation &attachmentsInformation)
    {
        attachmentsInformation.Rasterization.sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        attachmentsInformation.Rasterization.depthClampEnable           = VK_FALSE;
        attachmentsInformation.Rasterization.rasterizerDiscardEnable    = VK_FALSE;
        attachmentsInformation.Rasterization.polygonMode                = static_cast<VkPolygonMode>(programInformation.Parameters.PolygoneMode);
        attachmentsInformation.Rasterization.lineWidth                  = 1.0f;
        attachmentsInformation.Rasterization.cullMode                   = static_cast<VkCullModeFlagBits>(programInformation.Parameters.CullMode_);
        attachmentsInformation.Rasterization.frontFace                  = VK_FRONT_FACE_CLOCKWISE;
        attachmentsInformation.Rasterization.depthBiasEnable            = static_cast<VkBool32>(programInformation.Parameters.DepthBiasEnable);
        attachmentsInformation.Rasterization.depthBiasConstantFactor    = programInformation.Parameters.DepthBiasConstantFactor;    // Optional
        attachmentsInformation.Rasterization.depthBiasClamp             = programInformation.Parameters.DepthBiasClamp;             // Optional
        attachmentsInformation.Rasterization.depthBiasSlopeFactor       = programInformation.Parameters.DepthBiasSlopeFactor;       // Optional

        attachmentsInformation.Multisampling.sType                      = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        attachmentsInformation.Multisampling.sampleShadingEnable        = VK_FALSE;

        attachmentsInformation.Multisampling.rasterizationSamples       = programInformation.RenderPass_->GetSampleCount();

        attachmentsInformation.Multisampling.minSampleShading           = 1.0f; // Optional
        attachmentsInformation.Multisampling.pSampleMask                = nullptr; // Optional
        attachmentsInformation.Multisampling.alphaToCoverageEnable      = VK_FALSE; // Optional
        attachmentsInformation.Multisampling.alphaToOneEnable           = VK_FALSE; // Optional

        m_ColorBlendAttachment.colorWriteMask                           = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        m_ColorBlendAttachment.blendEnable                              = VK_TRUE;
        m_ColorBlendAttachment.srcColorBlendFactor                      = VK_BLEND_FACTOR_SRC_ALPHA;
        m_ColorBlendAttachment.dstColorBlendFactor                      = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        m_ColorBlendAttachment.colorBlendOp                             = VK_BLEND_OP_ADD;
        m_ColorBlendAttachment.srcAlphaBlendFactor                      = VK_BLEND_FACTOR_ONE;
        m_ColorBlendAttachment.dstAlphaBlendFactor                      = VK_BLEND_FACTOR_ZERO;
        m_ColorBlendAttachment.alphaBlendOp                             = VK_BLEND_OP_ADD;

        attachmentsInformation.ColorBlending.sType                      = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        attachmentsInformation.ColorBlending.logicOpEnable              = VK_FALSE;
        attachmentsInformation.ColorBlending.logicOp                    = VK_LOGIC_OP_COPY; // Optional
        attachmentsInformation.ColorBlending.attachmentCount            = 1;
        attachmentsInformation.ColorBlending.pAttachments               = &m_ColorBlendAttachment;
        attachmentsInformation.ColorBlending.blendConstants[0]          = 0.0f; // Optional
        attachmentsInformation.ColorBlending.blendConstants[1]          = 0.0f; // Optional
        attachmentsInformation.ColorBlending.blendConstants[2]          = 0.0f; // Optional
        attachmentsInformation.ColorBlending.blendConstants[3]          = 0.0f; // Optional

        attachmentsInformation.DepthStencil.sType                       = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        attachmentsInformation.DepthStencil.depthTestEnable             = programInformation.DepthTestEnable;
        attachmentsInformation.DepthStencil.depthWriteEnable            = programInformation.DepthWriteEnable;
        attachmentsInformation.DepthStencil.depthCompareOp              = VK_COMPARE_OP_LESS;
        attachmentsInformation.DepthStencil.depthBoundsTestEnable       = VK_FALSE;
        attachmentsInformation.DepthStencil.minDepthBounds              = 0.0f; // Optional
        attachmentsInformation.DepthStencil.maxDepthBounds              = 1.0f; // Optional
        attachmentsInformation.DepthStencil.stencilTestEnable           = VK_FALSE;
        attachmentsInformation.DepthStencil.front = {}; // Optional
        attachmentsInformation.DepthStencil.back = {}; // Optional

    }

    void IProgram::CreateUniformBuffers()
    {

        uint32_t index = 0;
        for (auto& descriptorInfo : m_DescriptorSetInfo) {

            for (uint32_t frame = 0; frame < MAX_GENERATED_FRAME; frame++) {

                BufferInformation bufferInfo{};
                bufferInfo.Usages  = descriptorInfo.Usages;
                bufferInfo.Flags   = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                bufferInfo.TotalSize = descriptorInfo.ElementCount * descriptorInfo.ElementStride;

                m_UniformBuffers.emplace_back();
                m_Buffers.emplace_back();
                if (descriptorInfo.Type == DescriptorType::COMBINED_IMAGE_SAMPLER || descriptorInfo.Type == DescriptorType::SAMPLED_IMAGE)
                {
                    index++;
                    continue;
                }
                m_UniformBuffers[index].Create(descriptorInfo.Name, bufferInfo);
                GPC_SUCCESS(m_UniformBuffers[index]
                    .Map<uint8_t>(&m_Buffers[index],
                        0, descriptorInfo.ElementCount * descriptorInfo.ElementStride));
                index++;

            }


        }

    }

    ErrorType IProgram::CreateDescriptorPool(ProgramInformation const &information)
    {
        std::vector<VkDescriptorPoolSize> poolSizes;
        for (auto binding : m_DescriptorSetInfo) {
            poolSizes.emplace_back(
                static_cast<VkDescriptorType>(binding.Type),
                binding.DescriptorCount * MAX_GENERATED_FRAME
            );
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_GENERATED_FRAME);

        if (vkCreateDescriptorPool(RenderDevice::GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
            return ErrorType::VULKAN_PROGRAM_POOL_CREATION;
        }

        return ErrorType::SUCCESS;
    }

    ErrorType IProgram::CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_GENERATED_FRAME, m_DescriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.pSetLayouts = layouts.data();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());

        m_DescriptorSets.resize(MAX_GENERATED_FRAME);
        if (vkAllocateDescriptorSets(RenderDevice::GetDevice(),
            &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
            return ErrorType::VULKAN_PROGRAM_ALLOCATION_CREATION;
        }
        return ErrorType::SUCCESS;
    }

    void IProgram::Destroy()
    {
        vkDestroyDescriptorSetLayout(RenderDevice::GetDevice(), m_DescriptorSetLayout, nullptr);

        vkDestroyPipelineLayout(RenderDevice::GetDevice(), m_PipelineLayout, nullptr);

        vkDestroyPipelineCache(RenderDevice::GetDevice(), m_PipelineCache, nullptr);

        vkDestroyPipeline(RenderDevice::GetDevice(), m_Pipeline, nullptr);

        vkDestroyDescriptorPool(RenderDevice::GetDevice(), m_DescriptorPool, nullptr);

        // Buffers
        for (int i = 0; i < m_UniformBuffers.size(); i++) {
            if (m_UniformBuffers[i].GetBuffer() == VK_NULL_HANDLE) continue;
            m_UniformBuffers[i].Unmap();
            m_UniformBuffers[i].Destroy();
        }
    }

    void IProgram::Update(int32_t binding, void const*data, uint32_t size, uint32_t offset) const
    {
        if (binding < 0) {
            return;
        }
        uint32_t index = binding*MAX_GENERATED_FRAME + RenderCommands::GetCurrentFrame();
        std::memcpy((m_Buffers[index] + (offset * size)), data, size);
    }


    int32_t IProgram::GetBindingSlot(std::string_view const& shaderName) const
    {
        if (!m_Bindings.contains(shaderName.data()))
            return -1;
        return m_Bindings.at(shaderName.data());
    }

    void IProgram::BindVertexInput(VertexInputInformation const &information)
    {

        m_BindingsDescription.push_back(information.BindingDescription);
        for (VkVertexInputAttributeDescription vertexDesc : information.AttributeDescriptions) {
            m_AttributeDescriptions.push_back(vertexDesc);
        }

    }


    void IProgram::BindLayout(DescriptorSetInformation const& information)
    {
        uint32_t index = m_DescriptorSetInfo.size();
        m_DescriptorSetInfo.push_back(information);
        m_Bindings[information.Name] = index;
    }

} // GPC