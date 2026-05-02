#include "GraphicsProgram.h"

#include "Geometry.h"
#include "RenderCommands.h"
#include "RenderDevice.h"
#include "RenderObject3D.h"
#include "RenderObject2D.h"
#include "RenderWindow.h"
#include "Sampler.h"
#include "Shader.h"
#include "Texture.h"

namespace GPC
{


    VkVertexInputBindingDescription Instance2D::GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(Instance2D);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> Instance2D::GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions (6);

        // layout(location(#location) = 0) in vec2(#format) inPosition;
        attributeDescriptions[0].binding = 1;
        attributeDescriptions[0].location = 3;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Instance2D, Tint);

        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 4;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Instance2D, Position);

        attributeDescriptions[2].binding = 1;
        attributeDescriptions[2].location = 5;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Instance2D, Size);

        attributeDescriptions[3].binding = 1;
        attributeDescriptions[3].location = 6;
        attributeDescriptions[3].format = VK_FORMAT_R32_UINT;
        attributeDescriptions[3].offset = offsetof(Instance2D, TextureIndex);

        attributeDescriptions[4].binding = 1;
        attributeDescriptions[4].location = 7;
        attributeDescriptions[4].format = VK_FORMAT_R32_UINT;
        attributeDescriptions[4].offset = offsetof(Instance2D, TextureLayerOffset);

        attributeDescriptions[5].binding = 1;
        attributeDescriptions[5].location = 8;
        attributeDescriptions[5].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(Instance2D, Rotation);

        return attributeDescriptions;
    }

    GraphicsProgram::~GraphicsProgram()
    = default;

    void GraphicsProgram::Create(ProgramInformation const &information)
    {

        m_ID = ID++;

        GPC_SUCCESS(CreateDescriptorLayout());
        GPC_SUCCESS(CreatePipelineLayout(information));

        VertexInformation vertexInformation;
        ViewportInformation viewportInformation;
        AttachmentsInformation attachmentsInformation;

        CreateVertex( vertexInformation);
        CreateViewport( viewportInformation);
        CreateAttachments(information, attachmentsInformation);

        GPC_SUCCESS(CreatePipeline(information, vertexInformation, viewportInformation, attachmentsInformation));

        GPC_SUCCESS(CreateDescriptorPool(information));
        GPC_SUCCESS(CreateDescriptorSets());
        CreateUniformBuffers();

    }

    void GraphicsProgram::Destroy()
    {
        IProgram::Destroy();
    }

    void GraphicsProgram::InitializeWith(std::vector<Texture*> const& textures, Sampler const& sampler)
    {

        m_imageInfos.resize(textures.size());
        for (int i = 0; i < textures.size(); i++) {
            m_imageInfos[i].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            m_imageInfos[i].imageView     = textures[i]->GetImageView();
            m_imageInfos[i].sampler       = sampler.GetSampler();
            textures[i]->SetUploadId(i);
        }


        for (int frame = 0; frame < MAX_GENERATED_FRAME; frame++) {


            std::vector<VkWriteDescriptorSet>   writeDescriptorSets(m_DescriptorSetInfo.size());
            std::vector<VkDescriptorBufferInfo> buffersInfo(m_DescriptorSetInfo.size());

            for (int i = 0; i < m_DescriptorSetInfo.size(); i++) {
                DescriptorSetInformation const& info = m_DescriptorSetInfo[i];

                writeDescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[i].dstSet = m_DescriptorSets[frame];
                writeDescriptorSets[i].descriptorType = static_cast<VkDescriptorType>(info.Type);
                writeDescriptorSets[i].dstBinding = info.BindingSlot;

                if (static_cast<VkDescriptorType>(info.Type) == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ) {
                    int32_t missingDesc = info.DescriptorCount - m_imageInfos.size();
                    for (int a = 0; a < missingDesc; a++) {
                        m_imageInfos.emplace_back( sampler.GetSampler(), textures[0]->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
                    }
                    writeDescriptorSets[i].pImageInfo = m_imageInfos.data();

                } else {
                    VkDescriptorBufferInfo& bufferInfo = buffersInfo.emplace_back();
                    bufferInfo.buffer = m_UniformBuffers[i*MAX_GENERATED_FRAME + frame].GetBuffer();
                    bufferInfo.offset = 0;
                    bufferInfo.range  = info.ElementCount * info.ElementStride;

                    writeDescriptorSets[i].pBufferInfo = &bufferInfo;
                }

                writeDescriptorSets[i].descriptorCount = info.DescriptorCount;

            }

            vkUpdateDescriptorSets(RenderDevice::GetDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

        }
    }

    void GraphicsProgram::InitializeWith(std::vector<VkDescriptorImageInfo>& images) const
    {


        for (int frame = 0; frame < MAX_GENERATED_FRAME; frame++) {


            std::vector<VkWriteDescriptorSet>   writeDescriptorSets(m_DescriptorSetInfo.size());
            std::vector<VkDescriptorBufferInfo> buffersInfo(m_DescriptorSetInfo.size());

            for (int i = 0; i < m_DescriptorSetInfo.size(); i++) {
                DescriptorSetInformation const& info = m_DescriptorSetInfo[i];

                writeDescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[i].dstSet = m_DescriptorSets[frame];
                writeDescriptorSets[i].descriptorType = static_cast<VkDescriptorType>(info.Type);
                writeDescriptorSets[i].dstBinding = info.BindingSlot;

                if (static_cast<VkDescriptorType>(info.Type) == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ) {
                    writeDescriptorSets[i].pImageInfo = images.data();

                } else {
                    VkDescriptorBufferInfo& bufferInfo = buffersInfo.emplace_back();
                    bufferInfo.buffer = m_UniformBuffers[i*MAX_GENERATED_FRAME + frame].GetBuffer();
                    bufferInfo.offset = 0;
                    bufferInfo.range  = info.ElementCount * info.ElementStride;

                    writeDescriptorSets[i].pBufferInfo = &bufferInfo;
                }

                writeDescriptorSets[i].descriptorCount = info.DescriptorCount;

            }

            vkUpdateDescriptorSets(RenderDevice::GetDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

        }
    }

    void GraphicsProgram::InitializeWith(int32_t binding, std::vector<Texture const*> const& textures, Sampler const& sampler) const
    {
        if (binding < 0) return;
        std::vector<VkDescriptorImageInfo> imageInfos(textures.size());
        for (int i = 0; i < textures.size(); i++) {
            if (textures[i] == nullptr)
                return;
            imageInfos[i].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[i].imageView     = textures[i]->GetImageView();
            imageInfos[i].sampler       = sampler.GetSampler();
        }

        DescriptorSetInformation const& info = m_DescriptorSetInfo[binding];

        for (uint32_t frame = 0; frame < MAX_GENERATED_FRAME; frame++) {

            std::vector<VkWriteDescriptorSet>   writeDescriptorSets(1);
            std::vector<VkDescriptorBufferInfo> buffersInfo(1);

            writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[0].dstSet = m_DescriptorSets[frame];
            writeDescriptorSets[0].descriptorType = static_cast<VkDescriptorType>(info.Type);
            writeDescriptorSets[0].dstBinding = info.BindingSlot;

            if (static_cast<VkDescriptorType>(info.Type) == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
                writeDescriptorSets[0].pImageInfo = imageInfos.data();
            }
            else {
                VkDescriptorBufferInfo& bufferInfo = buffersInfo.emplace_back();
                bufferInfo.buffer = m_UniformBuffers[info.BindingSlot*MAX_GENERATED_FRAME + frame].GetBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range  = info.ElementCount * info.ElementStride;

                writeDescriptorSets[0].pBufferInfo = &bufferInfo;
            }

            writeDescriptorSets[0].descriptorCount = info.DescriptorCount;


            vkUpdateDescriptorSets(RenderDevice::GetDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

        }

    }

    void GraphicsProgram::SetImages(std::vector<Texture const *> const &textures, Sampler const &sampler) const
    {
        // Find first non-null texture as fallback for null slots (avoids SIGSEGV).
        Texture const* fallback = nullptr;
        for (auto* t : textures) { if (t) { fallback = t; break; } }
        if (!fallback) return; // all null, nothing valid to bind

        std::vector<VkDescriptorImageInfo> imageInfos(textures.size());
        for (int i = 0; i < textures.size(); i++) {
            Texture const* tex = textures[i] ? textures[i] : fallback;
            imageInfos[i].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[i].imageView     = tex->GetImageView();
            imageInfos[i].sampler       = sampler.GetSampler();
        }

        for (uint32_t frame = 0; frame < MAX_GENERATED_FRAME; frame++) {

            std::vector<VkWriteDescriptorSet>   writeDescriptorSets(m_DescriptorSetInfo.size());
            std::vector<VkDescriptorBufferInfo> buffersInfo(m_DescriptorSetInfo.size());

            for (int i = 0; i < m_DescriptorSetInfo.size(); i++) {
                DescriptorSetInformation const& info = m_DescriptorSetInfo[i];

                writeDescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[i].dstSet = m_DescriptorSets[frame];
                writeDescriptorSets[i].descriptorType = static_cast<VkDescriptorType>(info.Type);
                writeDescriptorSets[i].dstBinding = info.BindingSlot;

                if (static_cast<VkDescriptorType>(info.Type) == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    int32_t missingDesc = info.DescriptorCount - imageInfos.size();
                    for (int a = 0; a < missingDesc; a++) {
                        imageInfos.emplace_back( sampler.GetSampler(), textures[0]->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
                    }
                    writeDescriptorSets[i].pImageInfo = imageInfos.data();

                } else {
                    VkDescriptorBufferInfo& bufferInfo = buffersInfo.emplace_back();
                    bufferInfo.buffer = m_UniformBuffers[i*MAX_GENERATED_FRAME + frame].GetBuffer();
                    bufferInfo.offset = 0;
                    bufferInfo.range  = info.ElementCount * info.ElementStride;

                    writeDescriptorSets[i].pBufferInfo = &bufferInfo;
                }

                writeDescriptorSets[i].descriptorCount = info.DescriptorCount;

            }

            vkUpdateDescriptorSets(RenderDevice::GetDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
        }
    }

    void GraphicsProgram::Update(int32_t binding, void const *data, uint32_t size, uint32_t offset) const
    {
        IProgram::Update(binding, data, size, offset);
    }

    ErrorType GraphicsProgram::CreatePipeline(ProgramInformation const &information, VertexInformation&vertexInformation, ViewportInformation const&viewportInformation,
                                              AttachmentsInformation const&attachmentsInformation)
    {

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        if (vkCreatePipelineCache(RenderDevice::GetDevice(),
            &pipelineCacheCreateInfo, nullptr, &m_PipelineCache) !=  VK_SUCCESS) {
            return ErrorType::VULKAN_PROGRAM_PIPELINE_CREATION;
        }

        std::vector<VkPipelineShaderStageCreateInfo> shaders;
        for (auto* shader : information.Shaders)
        {
            shaders.push_back(shader->GetShaderInformation());
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount             = static_cast<uint32_t>(shaders.size());
        pipelineInfo.pStages                = shaders.data();
        pipelineInfo.pVertexInputState      = &vertexInformation.InputCreateInfo;
        pipelineInfo.pInputAssemblyState    = &vertexInformation.AssemblyCreateInfo;
        pipelineInfo.pViewportState         = &viewportInformation.ViewportState;
        pipelineInfo.pDynamicState          = &viewportInformation.DynamicState;
        pipelineInfo.pRasterizationState    = &attachmentsInformation.Rasterization;
        pipelineInfo.pMultisampleState      = &attachmentsInformation.Multisampling;
        pipelineInfo.pDepthStencilState     = &attachmentsInformation.DepthStencil;
        pipelineInfo.pColorBlendState       = &attachmentsInformation.ColorBlending;
        pipelineInfo.layout                 = m_PipelineLayout;
        pipelineInfo.renderPass             = information.RenderPass_->GetRenderpass();
        pipelineInfo.flags                  = 0;
        pipelineInfo.subpass                = 0;
        pipelineInfo.basePipelineIndex      = -1;
        pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(RenderDevice::GetDevice(), m_PipelineCache, 1,
                                      &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
            return ErrorType::VULKAN_PROGRAM_PIPELINE_CREATION;
        }
        return ErrorType::SUCCESS;
    }

    ErrorType GraphicsProgram::CreatePipelineLayout(ProgramInformation const &information)
    {
        VkPushConstantRange constantRange{};
        constantRange.offset    = information.LayoutInformation.Offset;
        constantRange.size      = information.LayoutInformation.Size;
        constantRange.stageFlags= information.LayoutInformation.StageFlags;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount           = 1; // Optional
        pipelineLayoutInfo.pSetLayouts              = &m_DescriptorSetLayout; // Optional
        pipelineLayoutInfo.pushConstantRangeCount   = 1; // Optional
        pipelineLayoutInfo.pPushConstantRanges      = &constantRange; // Optional

        if (vkCreatePipelineLayout(RenderDevice::GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            return ErrorType::VULKAN_PROGRAM_BINDINGS_CREATION;
        }
        return ErrorType::SUCCESS;
    }

    void GraphicsProgram::UseShadows(RenderWindow* pWindow)
    {
        std::vector<Texture const*> spotLightDepth;
        spotLightDepth.push_back(pWindow->GetRenderTarget().GetSpotShadowDepthTexture());
        Sampler spotLightDepthSampler;
        spotLightDepthSampler.Create(SamplerInformation{});
        InitializeWith(GetBindingSlot("shadowSpotLightMapDepth"), spotLightDepth, spotLightDepthSampler);

        std::vector<Texture const*> pointLightDepth;
        pointLightDepth.push_back(pWindow->GetRenderTarget().GetShadowCubeMapDepthTexture());
        Sampler pointLightDepthSampler;
        pointLightDepthSampler.Create(SamplerInformation{});
        InitializeWith(GetBindingSlot("shadowPointLightMapDepth"), pointLightDepth, pointLightDepthSampler);

        std::vector<Texture const*> dirLightDepth;
        dirLightDepth.push_back(pWindow->GetRenderTarget().GetDirShadowDepthTexture());
        Sampler dirLightDepthSampler;
        dirLightDepthSampler.Create(SamplerInformation{});
        InitializeWith(GetBindingSlot("shadowDirLightMapDepth"), dirLightDepth, dirLightDepthSampler);
    }
}
