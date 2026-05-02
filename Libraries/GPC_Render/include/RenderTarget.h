#pragma once

#include <array>
#include <set>
#include <vector>
#include <glm/glm.hpp>

#include "Object.h"

#include "GPC_Framework.h"

#include "vulkan/vulkan.h"
#include "Font.h"
#include "Geometry.h"
#include "Light.h"
#include "Primitive.h"
#include "GraphicsProgram.h"
#include "RenderCommands.h"
#include "Texture.h"
#include "RenderDevice.h"
#include "RenderObject3D.h"
#include "RenderObject2D.h"
#include "RenderObjectBatched3D.h"
#include "RenderPass.h"
#include "Swapchain.h"
#include "Shader.h"

namespace GPC
{
    struct RenderObject3D;
    class GraphicsProgram;

    struct BlitInformation
    {
        Texture const *              Texture_;
        VkImageSubresourceLayers     TextureInformation;
        glm::ivec2                   SourceStart;
        glm::ivec2                   SourceEnd;
        glm::ivec2                   DestStart;
        glm::ivec2                   DestEnd;
    };

    struct RenderTargetInformation {
        uint32_t        Width   = 1080;
        uint32_t        Height  = 720;
        int32_t         OffsetX = 0;
        int32_t         OffsetY = 0;
        VkFormat        Format  = VK_FORMAT_UNDEFINED;
        ImageLayoutType Layout  = ImageLayoutType::PRESENT_SRC;

        bool            UseMSAA             = false;
        bool            UseDepthBuffer      = false;
        bool            UseBloom            = true;
        Swapchain*      Swapchain_           = { nullptr };
    };

    struct DrawObjectInformation
    {
        glm::mat4           Transform;
        Material const*     Material_;
        uint32_t            Index;
        VkBuffer            VertexBuffer;
        VkBuffer            IndexBuffer;
    };

    struct ShadowsUpdateInformation
    {
        GraphicsProgram const* Program;
        RenderPass*            RenderPass_;
    };

    GPC_INHERIT_OBJECT(RenderTarget)
    {
        struct Object2DInstance
        {
            glm::vec2       Position;
            glm::vec2       Size;
            uint32_t        TextureIndex;
            uint32_t        TextureLayerOffset;
        };

    public:
        RenderTarget() = default;
        ~RenderTarget() override;

        ErrorType Create(RenderTargetInformation const& information);
        void Destroy();

        void BeginDirShadows(const ShadowsUpdateInformation& information, const std::vector<Light*>& lights, glm::vec3 camPos, bool isSceneVillage);
        void EndDirShadows(const ShadowsUpdateInformation& information);

        void BeginSpotShadows(ShadowsUpdateInformation& information, std::vector<Light*> lights, glm::vec3 camPos);
        void EndSpotShadows(const ShadowsUpdateInformation& information);

        void BeginPointShadows(const ShadowsUpdateInformation& information, const std::vector<Light*>& lights, glm::vec3 camPos);
        void EndPointShadows(const ShadowsUpdateInformation& information, const std::vector<Light*>& lights);

        void BeginDraw3D();
        void EndDraw3D();

        void BeginDraw2D();
        void EndDraw2D();

        void UsePostEffect(GraphicsProgram const* program);

        void UseProgram(GraphicsProgram const* program);
        void EndPrograms();

        void DrawObjects(RenderObject3DBatched const& object);

        void DrawObject(RenderObject3D const& object);
        void DrawObject(RenderObject2D const& object);
        void DrawText(RenderText2D const & text);

        void BlitTexture(BlitInformation const& information);

        void UpdateCamera(glm::mat4 const& View, glm::mat4 const& Proj);
        void UpdateLights(const std::vector<Light*>& lights);
        void UpdateLight(const Light* light, uint32_t id);
        void UpdateShadows(const std::vector<Light*>& lights, glm::vec3 camPos, bool isSceneVillage);

        RenderPass&   GetRenderPass3D() { return m_RenderPass3D; }
        RenderPass&   GetRenderPass2D() { return m_RenderPass2D; };
        Texture*        GetImage() { return *(m_Images[RenderCommands::GetCurrentFrame()]); };
        Texture**       GetImage(uint32_t index) { return m_Images[index]; };
        std::vector<Texture**> const& GetImages() { return m_Images; };

    	Texture const* GetDirShadowDepthTexture() const { return m_DepthDirShadowTexture; };
    	Texture const* GetSpotShadowDepthTexture() const { return m_DepthSpotShadowTexture; };
    	Texture const* GetShadowCubeMapDepthTexture() const { return m_DepthShadowCubeTexture; };

        std::set<GraphicsProgram const*>    m_UsedPrograms;
        std::vector<GraphicsProgram const*> m_PostEffectProgram;

        std::unordered_map<Texture const*, uint32_t>    m_DrawSprite;
        std::unordered_map<Texture const*, uint32_t>    m_DrawText;

        std::vector<Texture const*> m_DrawTexturesSprite;
        std::vector<Texture const*> m_DrawTexturesText  ;

        uint32_t m_NextSpriteIndex = 0;
        uint32_t m_NextTextIndex = 0;

        std::array<Instance2D, MAX_ENTITIES>    m_2DDrawSprite;
        std::array<Instance2D, MAX_ENTITIES>    m_2DDrawText;

        uint32_t                                m_2DDrawSpriteCount = 0;
        uint32_t                                m_2DDrawTextCount = 0;

        void DestroyTextures();
        void Resize(RenderTargetInformation& information); // LIER A LA SWAPCHAIN

        UniformCameraBuffer const& GetCamera()
        {
            return m_3DCameraBuffer;
        }
    private:

        std::vector<VkClearValue>   m_ClearValues {};

        UniformCameraBuffer         m_2DCameraBuffer              = {};
        UniformCameraBuffer         m_3DCameraBuffer              = {};
        UniformLightBuffer          m_LightBuffer               = {};
        TransformData               m_TransformData             = {};
        MaterialData                m_MaterialData              = {};

        Object3DConstants           m_3DConstantBuffer  = {};

        RenderPass                  m_RenderPass3D;
        RenderPass                  m_RenderPassHDR;
        RenderPass                  m_RenderPassPostEffect;
        RenderPass                  m_RenderPass2D;

        VkExtent2D                  m_Extent            = { 1080, 720 };
        VkOffset2D                  m_Offset            = { 0, 0 };

        RenderTargetInformation     m_RenderInformation = {};

        std::vector<Texture**>      m_Images = std::vector<Texture**>(MAX_GENERATED_FRAME);

        // HDR
        Texture*                    m_HDRTexture;
        GraphicsProgram             m_HDRProgram = {};
        Shader                      m_HDRShaders[2];

        // Bloom
        GraphicsProgram             m_BloomProgram = {};
        Shader                      m_BloomShaders[2];

        // Depth buffer
        VkFormat                    m_DepthStencilFormat        = VK_FORMAT_D32_SFLOAT;
        Texture*                    m_DepthStencilTexture       = { nullptr };

        // MSAA
        VkSampleCountFlagBits       m_MsaaSampleCount           = VK_SAMPLE_COUNT_1_BIT;
        Texture*                    m_MsaaTexture               = { nullptr };;

        // 2D Render Target
        RenderObject2D              m_TextObject{};
        Material                    m_TextMaterial{};
        Sampler                     m_TextSampler{};
        Sampler                     m_SpriteSampler{};
        Geometry                    m_2DGeometry{};
        Shader                      m_2DBaseShader[4];

        GraphicsProgram             m_2DSpriteBaseProgram{};
        std::array<Buffer, MAX_GENERATED_FRAME>      m_2DSpriteBuffer{};
        std::array<Instance2D*, MAX_GENERATED_FRAME> mp_2DMappedSprite{};

        GraphicsProgram             m_2DTextBaseProgram         = {};
        std::array<Buffer, MAX_GENERATED_FRAME>      m_2DTextBuffer{};
        std::array<Instance2D*, MAX_GENERATED_FRAME> mp_2DMappedText{};

        // Batching
        Buffer                      m_Batched3DObjectBuffer{};
        Batched3DObject*            mp_MappedBatched3DObject    = { nullptr};

        uint64_t                    m_EntityDrawIndex   = 0;
        uint8_t                     m_AttachmentIndex   = 0;

        void        DrawHDRObjects();
        void        ApplyPostEffects();
        void        UpdateProgramValues();

        ErrorType   CreateRenderPass();

        void        CreateTextures();

        ErrorType   CreateRenderTexture();
        ErrorType   CreateHDRTexture();
        ErrorType   CreateMsaaResources() ;
        ErrorType   CreateHDRResources() ;
        ErrorType   CreatePostEffectResources() ;
        ErrorType   CreateDepthResources();
        ErrorType   CreateBatchingResources();
        void        Create2DFeatures();

        std::array<DrawObjectInformation, MAX_ENTITIES>  m_shadowObjects;
        std::array<DrawObjectInformation, MAX_ENTITIES>  m_HDRObjects;

        uint32_t m_shadowObjectsCount;
        uint32_t m_HDRObjectsCount;

        VkExtent2D                  m_shadowMapResolution = {2048, 2048};
        VkExtent2D                  m_shadowMapDirResolution = {8196, 8196};

        //DIRECTIONAL Light shadow informations
        RenderPass                  m_DirShadowRenderPass;
        GraphicsProgram             m_DirShadowProgram = {};
        Texture*                    m_DepthDirShadowTexture;
        Shader                      m_DirShadowBaseShader[3];
        std::vector<VkDescriptorImageInfo>  m_DirTextures {};

        //TODO Create a new struct for this one because we allocate 50 mat4x4 for directional
        //when we know we don't put that much in the game
        TransformLight              m_DirTransformBuffer = {};

        //SPOT Light shadow informations
        RenderPass                  m_SpotShadowRenderPass;
        GraphicsProgram             m_SpotShadowProgram = {};
        Texture*                    m_DepthSpotShadowTexture;
        Shader                      m_SpotShadowBaseShader[3];
        TransformLight              m_SpotTransformBuffer = {};


        //FOR SPOT and DIR
        bool m_HasDirectionalLight = false;
        TransformLight              m_DirSpotTransformBuffer = {};

        //POINT Light shadow informations
        RenderPass                  m_PointShadowRenderPass;
        GraphicsProgram             m_PointShadowProgram  = {};
        Texture*                    m_DepthShadowCubeTexture;
        Shader                      m_PointShadowShader[3];
        TransformLight              m_PointShadowBuffer = {};

        ShadowsUpdateInformation    m_shadowUpdateInformations[3];

        ErrorType CreateShadowRenderPass();
    };

} // GPC