#include "RenderPass.h"

#include "Defines.h"
#include "RenderCommands.h"
#include "RenderDevice.h"

namespace GPC
{
    Attachment& Attachment::BindTextures(std::vector<Texture*> const& textures)
    {
        for (auto* texture : textures) {
            BindTexture(&texture);
        }
        return *this;
    }

    Attachment & Attachment::BindTextures(std::vector<Texture **> textures)
    {
        for (auto texture : textures) {
            BindTexture(texture);
        }
        return *this;
    }

    Attachment & Attachment::BindTexture(Texture **texture)
    {
        m_ImageView.push_back(texture);
        return *this;
    }

    Attachment & Attachment::Format(VkFormat format)
    {
        m_Format = format;
        return *this;
    }

    Attachment & Attachment::Flag(VkAttachmentDescriptionFlags flag)
    {
        m_Flag = flag;
        return *this;
    }

    Attachment & Attachment::SampleCount(VkSampleCountFlagBits samples)
    {
        m_Sample = samples;
        return *this;
    }

    Attachment & Attachment::PassLayout(ImageLayoutType inLayout, ImageLayoutType outLayout)
    {
        m_InitialLayout = inLayout;
        m_FinalLayout = outLayout;
        return *this;
    }

    Attachment & Attachment::UseLayout(ImageLayoutType layout)
    {
        m_UsedLayout = layout;
        return *this;
    }

    Attachment & Attachment::UseClearValue(VkClearValue value)
    {
        m_ClearValue = value;
        return *this;
    }

    Attachment & Attachment::JustStoreNewColor()
    {

        m_LoadOp    = LoadOperation::DONT_CARE;
        m_StoreOp   = StoreOperation::STORE;
        return *this;

    }

    Attachment & Attachment::KeepPreviousColor(bool should)
    {
        if (!should)
            m_LoadOp = LoadOperation::CLEAR;
        else
            m_LoadOp = LoadOperation::LOAD;
        m_StoreOp = StoreOperation::STORE;
        return *this;
    }

    Attachment & Attachment::KeepDepthBuffer(bool should)
    {
        if (!should)
            m_LoadOp = LoadOperation::CLEAR;
        else
            m_LoadOp = LoadOperation::LOAD;
        m_StoreOp = StoreOperation::DONT_CARE;
        return *this;
    }

    void RenderPass::Create(RenderPassInformation const &information)
    {
        m_RenderPassInformation = information;

        GPC_SUCCESS(CreateRenderpass());

    }


    void RenderPass::Destroy()
    {
        DestroyFramebuffer();

        vkDestroyRenderPass(RenderDevice::GetDevice(), m_RenderPass, nullptr);
    }

    void RenderPass::DestroyFramebuffer()
    {
        for (auto framebuffer : m_Framebuffers)
            vkDestroyFramebuffer(RenderDevice::GetDevice(), framebuffer, nullptr);
    }

    void RenderPass::Resize(VkExtent2D extent)
    {
        m_RenderPassInformation.Extent = extent;

        CreateFramebuffers();
    }

    void RenderPass::Begin()
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass           = m_RenderPass;
        renderPassInfo.framebuffer          = m_Framebuffers[RenderCommands::GetCurrentFrame()];
        renderPassInfo.renderArea.offset    = m_RenderPassInformation.Offset;
        renderPassInfo.renderArea.extent    = m_RenderPassInformation.Extent;
        renderPassInfo.clearValueCount      = static_cast<uint32_t>(m_ClearValues.size());
        renderPassInfo.pClearValues         = m_ClearValues.data();

        vkCmdBeginRenderPass(RenderCommands::GetCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>( m_RenderPassInformation.Extent.width);
        viewport.height = static_cast<float>( m_RenderPassInformation.Extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(RenderCommands::GetCommandBuffer(), 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_RenderPassInformation.Extent;
        vkCmdSetScissor(RenderCommands::GetCommandBuffer(), 0, 1, &scissor);
    }

    void RenderPass::End()
    {
        vkCmdEndRenderPass(RenderCommands::GetCommandBuffer());
    }

    AttachmentIndex RenderPass::BindAttachment(Attachment const &attachments)
    {

        if (attachments.m_ReferenceLayout == AttachmentType::COLOR)
            m_SampleCount = attachments.m_Sample;

        VkAttachmentDescription& description = m_AttachmentDescriptions.emplace_back();
        description.flags           = attachments.m_Flag;
        description.format          = attachments.m_Format;
        description.samples         = attachments.m_Sample;

        description.initialLayout   = static_cast<VkImageLayout>(attachments.m_InitialLayout);
        description.finalLayout     = static_cast<VkImageLayout>(attachments.m_FinalLayout);

        description.loadOp          = static_cast<VkAttachmentLoadOp>(attachments.m_LoadOp);
        description.storeOp         = static_cast<VkAttachmentStoreOp>(attachments.m_StoreOp);
        description.stencilLoadOp   = static_cast<VkAttachmentLoadOp>(attachments.m_StencilLoadOp);
        description.stencilStoreOp  = static_cast<VkAttachmentStoreOp>(attachments.m_StencilStoreOp);

        VkAttachmentReference& reference = m_AttachmentReferences[attachments.m_ReferenceLayout].emplace_back();
        reference.attachment    = m_AttachmentIndex++;
        reference.layout        = static_cast<VkImageLayout>(attachments.m_UsedLayout);

        m_AttachedImageViews.push_back(attachments.m_ImageView);
        m_ClearValues.push_back(attachments.m_ClearValue);

        return {static_cast<uint32_t>(m_AttachmentDescriptions.size() - 1), attachments.m_ReferenceLayout};
    }

    VkRenderPass RenderPass::GetRenderpass() const
    {
        return m_RenderPass;
    }

    std::vector<VkFramebuffer> const & RenderPass::GetFramebuffers() const
    {
        return m_Framebuffers;
    }

    VkSampleCountFlagBits RenderPass::GetSampleCount() const
    {
        return m_SampleCount;
    }

    VkAttachmentDescription & RenderPass::GetAttachmentDescription(AttachmentIndex const& i)
    {
        return m_AttachmentDescriptions[i.Index];
    }

    VkAttachmentReference & RenderPass::GetAttachmentReference(AttachmentIndex const &i)
    {
        return m_AttachmentReferences[i.Type][i.Index];
    }

    bool RenderPass::ContainsLayout(AttachmentType type) const
    {
        return m_AttachmentReferences.contains(type);
    }

    ErrorType RenderPass::CreateRenderpass()
    {

        VkSubpassDescription subpass {};

        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = m_AttachmentReferences[AttachmentType::COLOR].size();
        if (ContainsLayout(AttachmentType::COLOR))
            subpass.pColorAttachments       = m_AttachmentReferences[AttachmentType::COLOR].data();
        if (ContainsLayout(AttachmentType::DEPTH_STENCIL))
            subpass.pDepthStencilAttachment = m_AttachmentReferences[AttachmentType::DEPTH_STENCIL].data();
        if (ContainsLayout(AttachmentType::RESOLVER))
            subpass.pResolveAttachments     = m_AttachmentReferences[AttachmentType::RESOLVER].data();

        VkSubpassDependency dependency{};
        dependency.srcSubpass       = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass       = 0;
        dependency.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount      = static_cast<uint32_t>(m_AttachmentDescriptions.size());
        renderPassInfo.pAttachments         = m_AttachmentDescriptions.data();

        renderPassInfo.subpassCount         = 1;
        renderPassInfo.pSubpasses           = &subpass;
        renderPassInfo.dependencyCount      = 1;
        renderPassInfo.pDependencies        = &dependency;

        if (vkCreateRenderPass(RenderDevice::GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            return ErrorType::VULKAN_RENDER_TARGET_CREATION;
        }

        RenderDevice::SetDebugName(VK_OBJECT_TYPE_RENDER_PASS, m_RenderPassInformation.Name, reinterpret_cast<uint64_t>(m_RenderPass));

        return ErrorType::SUCCESS;
    }

    ErrorType RenderPass::CreateFramebuffers()
    {
        for (int frame = 0; frame < MAX_GENERATED_FRAME; frame++) {

            std::vector<VkImageView> attachments;

            for (auto& imageViewsVector : m_AttachedImageViews) {
                attachments.push_back((*imageViewsVector[frame%imageViewsVector.size()])->GetImageView());
            }

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = m_RenderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments    = attachments.data();
            framebufferInfo.width           = m_RenderPassInformation.Extent.width + m_RenderPassInformation.Offset.x;
            framebufferInfo.height          = m_RenderPassInformation.Extent.height + m_RenderPassInformation.Offset.y;
            framebufferInfo.layers          = m_RenderPassInformation.NumberOfFrameBufferLayer;

            if (vkCreateFramebuffer(RenderDevice::GetDevice(), &framebufferInfo, nullptr, &m_Framebuffers[frame]) != VK_SUCCESS) {
                return ErrorType::VULKAN_RENDER_FRAMEBUFFER_CREATION;
            }

        }
        return ErrorType::SUCCESS;
    }
}
