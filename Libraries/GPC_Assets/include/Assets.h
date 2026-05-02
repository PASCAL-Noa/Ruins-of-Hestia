#pragma once

#include "../../GPC_Render/include/Render.h"

#include <vector>

#include "Blob.h"
#include "Meta/MetaTypes.h"

#define GPC_ASSETS GPC::Assets::GetInstance()

#define GPC_TEXTURE(name) GPC_ASSETS->GetAssets(name)->ToTexture()
#define GPC_MESH(name) &GPC_ASSETS->GetAssets(name)->ToMesh()->MeshGeometry
#define GPC_POINT_LIGHT(name) GPC_ASSETS->GetAssets(name)->ToLight()->ToPointLight()
#define GPC_SPOT_LIGHT(name) GPC_ASSETS->GetAssets(name)->ToLight()->ToSpotLight()
#define GPC_DIRECTIONAL_LIGHT(name) GPC_ASSETS->GetAssets(name)->ToLight()->ToDirectionalLight()
#define GPC_TEXTURE_ARRAY(array_name) GPC_ASSETS->GetAssets(array_name)->ToTextureArray()
#define GPC_TEXTURE_ID(texture_array, tex_name) GPC_ASSETS->GetAssets(texture_array)->ToTextureArray()->GetTextureID(tex_name)
#define GPC_SHADER(name) GPC_ASSETS->GetAssets(name)->ToShader()
#define GPC_GRAPHIC_PROGRAM(name) GPC_ASSETS->GetAssets(name)->ToGraphicProgram()
#define GPC_COMPUTE_PROGRAM(name) GPC_ASSETS->GetAssets(name)->ToComputeProgram()
#define GPC_AUDIO(name) GPC_ASSETS->GetAssets(name)->ToAudioResource()
#define GPC_FONT(name) &GPC_ASSETS->GetAssets(name)->ToFont()->Font

namespace GPC
{
    class Loader;

    struct FontAsset;
    struct TextureAsset;
    struct TextureArrayAsset;
    struct MeshAsset;
    struct LightAsset;
    struct ShaderAsset;
    struct GraphicProgramAsset;
    struct ComputeProgramAsset;

    struct SpriteSheetAsset;
    struct Model3DAsset;
    struct AnimationAsset;
    struct AudioResourceAsset;

    using AssetID = int32_t;
    struct Asset
    {
        AssetID Id;
        std::string Name;

        TextureAsset* ToTexture() { return pointer_cast<TextureAsset*>(this); }
        TextureArrayAsset* ToTextureArray() { return pointer_cast<TextureArrayAsset*>(this); }
        MeshAsset* ToMesh() { return pointer_cast<MeshAsset*>(this); }
        LightAsset* ToLight() { return pointer_cast<LightAsset*>(this); }
        ShaderAsset* ToShader() { return pointer_cast<ShaderAsset*>(this); }
        GraphicProgramAsset* ToGraphicProgram() { return pointer_cast<GraphicProgramAsset*>(this); }
        ComputeProgramAsset* ToComputeProgram() { return pointer_cast<ComputeProgramAsset*>(this); }
        AudioResourceAsset* ToAudioResource() { return pointer_cast<AudioResourceAsset*>(this); }
        FontAsset* ToFont() { return pointer_cast<FontAsset*>(this); }

        virtual ~Asset() = default;
        virtual void Destroy() = 0;

    };

    struct TextureAsset : public Asset {
        uint32_t Width, Height;
        uint32_t Channels;
        Blob Data;

        [[nodiscard]] ImageInformation GetImageInformation() const;
        void Destroy() override {};
    };

    struct TextureArrayAsset : public Asset {
        TextureArray Array;
        Sampler TextureSampler;
        std::unordered_map<std::string, std::pair<TextureAsset*, TextureID>> PairAssetID;
        uint32_t Id;

        TextureID GetTextureID(const std::string &name) ;
        bool HasTexture(const std::string &name) const { return PairAssetID.contains(name); }
        void Destroy() override;
    };

    struct MeshAsset : public Asset {
        Geometry MeshGeometry;
        GeometryData* pGeometryData = nullptr;
        void Destroy() override;
    };

    struct LightDescriptor {
        Light::LightType Type = Light::LightType::POINT;
        Color EmissionColor = { 255, 255, 255, 255 };
        Color AmbientColor = { 0, 0, 0, 255 };
        float Intensity = 10.0f;

        float Range = 20.0f;
        glm::vec4 Direction = { 0, -1, 0, 1};
        float InnerCone = std::cos(glm::radians(50.0f));
        float OuterCone = std::cos(glm::radians(70.0f));
    };

    struct LightAsset : public Asset {
        GPC::Light* pLight;

        [[nodiscard]] inline PointLight* ToPointLight() const { return static_cast<PointLight*>(static_cast<void*>(pLight)); }
        [[nodiscard]] inline SpotLight* ToSpotLight() const { return static_cast<SpotLight*>(static_cast<void*>(pLight)); }
        [[nodiscard]] inline DirectionalLight* ToDirectionalLight() const { return static_cast<DirectionalLight*>(static_cast<void*>(pLight)); }
        void Destroy() override;
    };

    struct FontAsset : public Asset {
        Font Font_;

        void Destroy() override;
    };


    struct ShaderAsset : public Asset {
        Shader::Type Type;
        Shader ShaderCode;

        void Destroy() override;
    };

    struct GraphicProgramAsset : public Asset {
        ShaderAsset* pVertex;
        ShaderAsset* pFragment;
        GraphicsProgram Program;

        void Destroy() override;
    };

    struct ComputeProgramAsset : public Asset {
        ShaderAsset* pCompute;
        ComputeProgram Program;

        void Destroy() override;
    };

    struct AudioResourceAsset : Asset {
        std::vector<float> Samples{};
        uint32_t           SampleRate{};
        uint16_t           Channels{};
        float              Duration{};

        void Destroy() override {};
    };

    class MetaLoader;

    class Assets
    {
    public:
        Assets();
        ~Assets();

        static Assets* GetInstance();
        static void Create(RenderWindow* pWindow);
        static void DestroyAllAssets();
        static void DestroyAsset(const std::string& name);

        // Functions utilisé par un gameplay
        Asset* AddAsset(Asset* asset);
        void RemoveAsset(const std::string& name);
        Asset* GetAssets(const std::string& name);
        ShaderAsset* GetShader(const std::string& name);
        [[nodiscard]] bool HasAsset(const std::string& name) const;

        TextureAsset* AddTexture(const std::string& name,const std::string& filePath);
        TextureAsset* AddTexture(const std::string &name, uint8_t* pData, uint32_t width, uint32_t height);
        TextureArrayAsset* AddTextureArray(const std::string& name, uint32_t width, uint32_t height, const std::vector<TextureAsset*>& pAssets);
        FontAsset* AddFont(const std::string& name,const std::string& filePath, uint32_t fontSize);
        MeshAsset* AddMesh(const std::string& name, GeometryData* pPrimitive);
        LightAsset* AddLight(const std::string& name, const LightDescriptor& descriptor);
        ShaderAsset* AddShader(const std::string& name, const std::string& path, Shader::Type type);

        ComputeProgramAsset* AddParticleProgram(
            const std::string& name,
            ParticleComputeInformation const& pInfo,
            ShaderAsset* compute
            );

        GraphicProgramAsset* AddGraphicProgram(
            const std::string& name,
            ShaderAsset* vertex,
            ShaderAsset* fragment,
            CullMode cull_mode,
            PolygonMode polygon_mode,
            const std::vector<VertexInputInformation> &vertex_input_info,
            const std::vector<DescriptorSetInformation>& descriptor_set_infos,
            RenderPassType type,
            RenderWindow* pWindow);

        AudioResourceAsset* AddAudio(const std::string& name, const std::string& filePath);

        MetaError LoadMeta(const std::string& metaPath);
        MetaError ResolveRuntimeAssets(const MetaRuntimeContext& ctx);
        [[nodiscard]] bool HasPendingRuntimeAssets() const;

        // AssetID AddAnimation(const std::string& name, float duration);
        // AssetID AddSprite(const std::string& name, const std::string& textureName, float uMin, float vMin, float uMax, float vMax);
        // AssetID AddSpriteSheet(const std::string& name, const std::string& textureName, uint32_t fWidth, uint32_t fHeight);
        // AssetID AddModel3D(const std::string& name, const std::string& meshName, const std::string& diffuseName);
        // AssetID AddAnimationFromSheet(const std::string& animName, const std::string& sheetName, float frameDuration);
        // void LoadAtlas(const std::string& filePath);

    private:
        void FreeAssets();

    protected:
        friend class AudioSystem;
        // Variable
        std::unordered_map<std::string, Asset*> m_Assets;
        MetaLoader* m_pMetaLoader = nullptr;

        // TODO : Add a Create to Get a reference to the window
        RenderWindow* mp_Window;
    };


}
