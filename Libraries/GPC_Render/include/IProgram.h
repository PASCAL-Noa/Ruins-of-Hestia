#pragma once
#include "Light.h"
#include "Material.h"
#include "RenderCommands.h"
#include "RenderPass.h"
#include "Sampler.h"

#include <glm/glm.hpp>

namespace GPC
{

    struct UniformCameraBuffer {
        glm::mat4 ViewProj;
        glm::mat4 View;
        glm::mat4 Proj;
    };

    struct UniformLightBuffer
    {
        alignas(16) uint32_t lightCount;
        LightData LightInfo[MAX_LIGHTS];
    };

    struct TransformLight
    {
        glm::mat4   Transform[MAX_LIGHTS];
    };

    struct TransformData
    {
        glm::mat4   Transform[MAX_ENTITIES];
    };

    struct MaterialData
    {
        Material    Material_[MAX_ENTITIES];               // METTRE DANS 2 STORAGE
    };

    enum class PolygonMode : uint8_t{
        MODE_FILL = 0,
        MODE_LINE = 1,
        MODE_POINT = 2,
    };

    enum class CullMode : uint8_t{
        FRONT= 0x00000001,
        BACK = 0x00000002,
        FRONT_AND_BACK = 0x00000003,
    };

    enum class DescriptorType : uint8_t {
        SAMPLER                = VK_DESCRIPTOR_TYPE_SAMPLER               ,
        COMBINED_IMAGE_SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        SAMPLED_IMAGE          = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE         ,
        STORAGE_IMAGE          = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE         ,
        UNIFORM_BUFFER         = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER        ,
        STORAGE_BUFFER         = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        ,

        END
    };
    typedef uint32_t DescriptorUsageBits;

    struct DescriptorSetInformation
    {
        std::string         Name;
        DescriptorType      Type;
        DescriptorUsageBits Usages;
        uint32_t            BindingSlot     = 0;
        uint32_t            DescriptorCount = 0;
        uint32_t            ElementCount    = 0;
        uint32_t            ElementStride   = 0;
        VkShaderStageFlags  Flags;
        Sampler const*      Sampler_;

        static DescriptorSetInformation ForCamera(uint32_t binding);
        static DescriptorSetInformation ForLights(uint32_t binding);
        static DescriptorSetInformation ForTransform(uint32_t binding);
        static DescriptorSetInformation ForMaterials(uint32_t binding);
        static DescriptorSetInformation ForImages(uint32_t binding, std::string const& id, uint32_t descriptorCount);
        static DescriptorSetInformation ForTextures(uint32_t binding);
    };

    struct AttachmentsParameters
    {
        PolygonMode PolygoneMode;
        CullMode    CullMode_;
        bool        DepthBiasEnable         = false;
        float       DepthBiasConstantFactor = 0.0f;
        float       DepthBiasClamp          = 0.0f;
        float       DepthBiasSlopeFactor    = 0.0f;
    };

    struct VertexInputInformation
    {
        VkVertexInputBindingDescription                 BindingDescription;
        std::vector<VkVertexInputAttributeDescription>  AttributeDescriptions;
    };

    struct PipelineLayoutInformation
    {
        uint32_t Offset                     = 0;
        uint32_t Size                       = 4;
        VkShaderStageFlags    StageFlags    = VK_SHADER_STAGE_VERTEX_BIT;
    };

    struct ProgramInformation
    {
        std::vector<Shader*>                        Shaders             {};
        AttachmentsParameters                       Parameters          {};
        RenderPass const*                           RenderPass_          {};
        PipelineLayoutInformation                   LayoutInformation   {};
        bool                                        DepthTestEnable     { true };
        bool                                        DepthWriteEnable    { true };
    };

    struct VertexInformation
    {
        VkPipelineVertexInputStateCreateInfo        InputCreateInfo     {};
        VkPipelineInputAssemblyStateCreateInfo      AssemblyCreateInfo  {};
    };

    struct ViewportInformation
    {
        VkPipelineDynamicStateCreateInfo            DynamicState        {};
        VkPipelineViewportStateCreateInfo           ViewportState       {};
    };

    struct AttachmentsInformation
    {
        VkPipelineRasterizationStateCreateInfo  Rasterization   {};
        VkPipelineMultisampleStateCreateInfo    Multisampling   {};
        VkPipelineColorBlendStateCreateInfo     ColorBlending   {};
        VkPipelineDepthStencilStateCreateInfo   DepthStencil    {};
    };

    class IProgram
    {

    public:
        virtual void Create(ProgramInformation const& information) = 0;
        virtual void Destroy();

        virtual void Update(int32_t binding, void const* data, uint32_t size, uint32_t offset) const;

        [[nodiscard]] VkPipeline const&         GetPipeline() const { return m_Pipeline; };
        [[nodiscard]] VkPipelineLayout const&   GetLayout() const { return m_PipelineLayout; };
        [[nodiscard]] VkDescriptorSet const&    GetDescriptorSet() const{ return m_DescriptorSets[RenderCommands::GetCurrentFrame()]; };
        [[nodiscard]] Buffer const&             GetBuffer(uint32_t index) const{ return m_UniformBuffers[index]; };

        int32_t GetBindingSlot(std::string_view const& shaderName) const;

        void BindVertexInput(VertexInputInformation const& information);
        void BindLayout(DescriptorSetInformation const& information);

    protected:

        VkPipelineCache                     m_PipelineCache         { VK_NULL_HANDLE };
        VkPipelineLayout                    m_PipelineLayout        { VK_NULL_HANDLE };
        VkPipeline                          m_Pipeline      { VK_NULL_HANDLE };

        VkDescriptorPool                    m_DescriptorPool        { VK_NULL_HANDLE };
        VkDescriptorSetLayout               m_DescriptorSetLayout   { VK_NULL_HANDLE };

        std::vector<VkDescriptorSet>                m_DescriptorSets        { VK_NULL_HANDLE };

        std::unordered_map<std::string, uint32_t>   m_Bindings;

        std::vector<DescriptorSetInformation>       m_DescriptorSetInfo{ };

        std::vector<Buffer>                         m_UniformBuffers   { };
        std::vector<uint8_t*>                       m_Buffers          { nullptr };

        VkPipelineColorBlendAttachmentState m_ColorBlendAttachment  {};

        std::vector<VkDynamicState>         m_DynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        std::vector<VkVertexInputBindingDescription>                m_BindingsDescription;
        std::vector<VkVertexInputAttributeDescription>              m_AttributeDescriptions;

        virtual ErrorType CreatePipelineLayout(ProgramInformation const &information) = 0;
        ErrorType CreateDescriptorLayout();

        void CreateVertex( VertexInformation& vertexInformation) const;
        void CreateViewport( ViewportInformation& viewportInformation) const;
        void CreateAttachments(ProgramInformation const& programInformation, AttachmentsInformation& attachmentsInformation);

        void CreateUniformBuffers();
        ErrorType CreateDescriptorPool(ProgramInformation const &information);
        ErrorType CreateDescriptorSets();
    };
} // GPC