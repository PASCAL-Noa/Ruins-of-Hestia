#pragma once

#include <vulkan/vulkan.h>

#include "Defines.h"
#include "ErrorType.h"
#include "Object.h"
#include "Texture.h"

namespace GPC
{
    class RenderPass;

    enum class ImageLayoutType : uint32_t {
        UNDEFINED   = VK_IMAGE_LAYOUT_UNDEFINED,
        GENERAL     = VK_IMAGE_LAYOUT_GENERAL,
        COLOR_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        DEPTH_STENCIL_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        DEPTH_STENCIL_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        SHADER_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        TRANSFER_SRC_OPTIMAL = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        TRANSFER_DST_OPTIMAL = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        PRESENT_SRC = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    enum class LoadOperation : uint8_t {
        LOAD      = VK_ATTACHMENT_LOAD_OP_LOAD     ,
        CLEAR     = VK_ATTACHMENT_LOAD_OP_CLEAR    ,
        DONT_CARE = VK_ATTACHMENT_LOAD_OP_DONT_CARE,

        END,
    };

    enum class StoreOperation : uint8_t {
        STORE     = VK_ATTACHMENT_STORE_OP_STORE    ,
        DONT_CARE = VK_ATTACHMENT_STORE_OP_DONT_CARE,

        END,
    };

    enum class AttachmentType : uint8_t {
        COLOR,
        DEPTH_STENCIL,
        RESOLVER,

        END,
    };

    enum RenderPassType
    {
        GRAPHICS_3D,
        GRAPHICS_2D,
        COMPUTE,
    };


    struct AttachmentIndex
    {
        uint32_t        Index;
        AttachmentType  Type;
    };

    struct ColorAttachmentInformation
    {
        LoadOperation   LoadOperation_;
        ImageLayoutType Layout;
        VkFormat        Format;
    };

    GPC_INHERIT_OBJECT(Attachment)
    {
    public:
        Attachment(AttachmentType attachedLayout) : m_Format(), m_Flag(0), m_Sample(), m_ClearValue(),
                                                    m_ReferenceLayout(attachedLayout)
        {
        };
        ~Attachment() override = default;

        Attachment& BindTextures(std::vector<Texture*> const& textures);
        Attachment& BindTextures(std::vector<Texture**> textures);
        Attachment& BindTexture(Texture ** texture);

        Attachment& Format(VkFormat format);
        Attachment& Flag(VkAttachmentDescriptionFlags flag);
        Attachment& SampleCount(VkSampleCountFlagBits samples);

        Attachment& PassLayout(ImageLayoutType inLayout, ImageLayoutType outLayout);
        Attachment& UseLayout(ImageLayoutType layout);
        Attachment& UseClearValue(VkClearValue value);

        Attachment& JustStoreNewColor();
        Attachment& KeepPreviousColor(bool shouldClear);
        Attachment& KeepDepthBuffer(bool shouldClear);

    private:
        VkFormat                        m_Format;
        VkAttachmentDescriptionFlags    m_Flag {};
        VkSampleCountFlagBits           m_Sample;
        VkClearValue                    m_ClearValue;

        std::vector<Texture**>        m_ImageView;

        LoadOperation       m_LoadOp            = LoadOperation::DONT_CARE;
        StoreOperation      m_StoreOp           = StoreOperation::DONT_CARE;

        LoadOperation       m_StencilLoadOp     = LoadOperation::DONT_CARE;
        StoreOperation      m_StencilStoreOp    = StoreOperation::DONT_CARE;

        ImageLayoutType     m_InitialLayout     = ImageLayoutType::UNDEFINED;
        ImageLayoutType     m_FinalLayout       = ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL;

        ImageLayoutType     m_UsedLayout       = ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL;

        AttachmentType      m_ReferenceLayout   = AttachmentType::COLOR;

        friend RenderPass;

    };

    struct RenderPassInformation
    {
        std::string Name;

        VkExtent2D  Extent = { 1080, 720 };
        VkOffset2D  Offset = { 0, 0 };

        uint32_t    NumberOfFrameBufferLayer = 1;
    };

    GPC_INHERIT_OBJECT(RenderPass)
    {
    public:
        RenderPass() = default;
        ~RenderPass() override = default;

        void Create(RenderPassInformation const& information);
        void Destroy();
        void DestroyFramebuffer();

        void Resize(VkExtent2D extent);

        void Begin();
        void End();

        AttachmentIndex BindAttachment(Attachment const& loadOperation);

        VkRenderPass                        GetRenderpass() const;
        std::vector<VkFramebuffer> const&   GetFramebuffers() const;
        VkSampleCountFlagBits               GetSampleCount() const;

        VkAttachmentDescription& GetAttachmentDescription(AttachmentIndex const& i);
        VkAttachmentReference& GetAttachmentReference(AttachmentIndex const& i);

    private:

        bool ContainsLayout(AttachmentType type) const;
        ErrorType CreateRenderpass();
        ErrorType CreateFramebuffers();

        std::vector<VkAttachmentDescription>                                        m_AttachmentDescriptions;
        std::unordered_map<AttachmentType, std::vector<VkAttachmentReference>>      m_AttachmentReferences;
        std::vector<std::vector<Texture**>>                                    m_AttachedImageViews;
        std::vector<VkClearValue>                                                   m_ClearValues;
        uint32_t                        m_AttachmentIndex = 0;

        RenderPassInformation           m_RenderPassInformation;

        VkRenderPass                    m_RenderPass;
        std::vector<VkFramebuffer>      m_Framebuffers = std::vector<VkFramebuffer>(MAX_GENERATED_FRAME);
        VkSampleCountFlagBits           m_SampleCount  = VK_SAMPLE_COUNT_1_BIT;

    };

}
