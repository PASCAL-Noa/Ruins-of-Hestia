#include "Assets.h"

#include "ArchiveManager.h"
#include <iostream>

#include "Loader.h"
#include "Meta/MetaLoader.h"
#include "WavParser.h"
#include "../../GPC_Engine/include/AnimationComponent.h"

namespace GPC
{
    ImageInformation TextureAsset::GetImageInformation() const {
        ImageInformation info;
        info.Width = Width;
        info.Height = Height;
        info.Stride = Channels;
        info.Data = Data.Ptr();
        return info;
    }

    TextureID TextureArrayAsset::GetTextureID(const std::string &name) {
        if (!PairAssetID.contains(name))
        {
            GPC_WARNING << "TextureArrayAsset::GetTextureID - texture not found: '" << name << "'" << ENDL;
            return 0;
        }
        return PairAssetID[name].second;
    }

    void TextureArrayAsset::Destroy()
    {
        TextureSampler.Destroy();
        Array.Destroy();
    }

    void MeshAsset::Destroy()
    {
        MeshGeometry.Destroy();
    }

    void LightAsset::Destroy()
    {
        pLight->Destroy();
        delete pLight;
        pLight = nullptr;
    }

    void FontAsset::Destroy()
    {
        Font_.Destroy();
    }

    void ShaderAsset::Destroy()
    {
        ShaderCode.Destroy();
    }

    void GraphicProgramAsset::Destroy()
    {
        Program.Destroy();
    }

    void ComputeProgramAsset::Destroy()
    {
        Program.Destroy();
    }

    Assets::Assets() : m_Assets() { }

    Assets::~Assets() {
        FreeAssets();
        delete m_pMetaLoader;
    }

    Assets* Assets::GetInstance()
    {
        static Assets instance;
        return &instance;
    }

    void Assets::Create(RenderWindow *pWindow) {
        GetInstance()->mp_Window = pWindow;
    }

    void Assets::DestroyAsset(const std::string &name)
    {
        auto& asset = GetInstance()->m_Assets[name];
        asset->Destroy();
        delete asset;
        asset = nullptr;
    }

    void Assets::DestroyAllAssets()
    {
        for (auto& m_asset : GetInstance()->m_Assets) {
            m_asset.second->Destroy();
            delete m_asset.second;
            m_asset.second = nullptr;
        }
    }

    Asset* Assets::AddAsset(Asset* asset)
    {
        DEBUG_ASSERT(asset != nullptr, "Tentative d'ajout d'un asset nul dans le manager !" + asset->Name);

        asset->Id = static_cast<uint32_t>(m_Assets.size()) + 1;
        m_Assets[asset->Name] = asset;

        return asset;
    }

    void Assets::RemoveAsset(const std::string& name)
    {
        auto it = m_Assets.find(name);
        DEBUG_ASSERT(it != m_Assets.end(), "Tentative de suppression d'un asset inexistant : " + name);
        if (it != m_Assets.end()) {
            delete it->second;
            m_Assets.erase(it);
        }
    }

    Asset* Assets::GetAssets(const std::string& name)
    {
        auto it = m_Assets.find(name);
        if (it != m_Assets.end()) {
            return it->second;
        }
        GPC_WARNING << "FAILED TO GET ASSET : " << name << ENDL;
        return nullptr;
    }

    ShaderAsset* Assets::GetShader(const std::string& name)
    {
        Asset* asset = GetAssets(name);
        return asset ? asset->ToShader() : nullptr;
    }

    bool Assets::HasAsset(const std::string& name) const
    {
        return m_Assets.find(name) != m_Assets.end();
    }

    TextureAsset* Assets::AddTexture(const std::string& name,const std::string& filepath)
    {
        if (m_Assets.contains(name)) return pointer_cast<TextureAsset*>(m_Assets[name]);
        TextureAsset* tex = new TextureAsset();
        tex->Name = name;

        if (!GPC::Loader::LoadTexture(filepath, tex)) {
            delete tex;
            return nullptr;
        }

        return pointer_cast<TextureAsset*>(AddAsset(tex));
    }

    TextureAsset* Assets::AddTexture(const std::string &name, uint8_t* pData, uint32_t width, uint32_t height) {
        if (m_Assets.contains(name)) return pointer_cast<TextureAsset*>(m_Assets[name]);
        TextureAsset* tex = new TextureAsset();
        tex->Name = name;

        uint64_t totalSize = static_cast<uint64_t>(width) * height * RGBA;
        tex->Data.Resize(totalSize);
        memcpy(tex->Data.Ptr(), pData, totalSize);

        tex->Height = height;
        tex->Width = width;
        tex->Channels = RGBA;

        return pointer_cast<TextureAsset*>(AddAsset(tex));
    }

    MeshAsset* Assets::AddMesh(const std::string& name, GeometryData* pPrimitive)
    {
        if (m_Assets.contains(name)) return pointer_cast<MeshAsset*>(m_Assets[name]);
        MeshAsset* mesh = new MeshAsset();
        mesh->Name = name;
        mesh->MeshGeometry.Create(pPrimitive);
        mesh->pGeometryData = pPrimitive;
        return pointer_cast<MeshAsset*>(AddAsset(mesh));
    }

    LightAsset* Assets::AddLight(const std::string &name, const LightDescriptor &descriptor) {
        if (m_Assets.contains(name)) return pointer_cast<LightAsset*>(m_Assets[name]);
        LightAsset* light = new LightAsset();
        light->Name = name;

        switch (descriptor.Type) {
            default:
            case Light::LightType::POINT: {
                auto* point = new PointLight();
                point->Create();

                point->SetColor(descriptor.EmissionColor);
                point->SetAmbient(descriptor.AmbientColor);
                point->SetRange(descriptor.Range);
                point->SetIntensity(descriptor.Intensity);

                light->pLight = point;
                break;
            }
            case Light::LightType::SPOT: {
                auto* spot = new SpotLight();
                spot->Create();

                spot->SetColor(descriptor.EmissionColor);
                spot->SetAmbient(descriptor.AmbientColor);
                spot->SetIntensity(descriptor.Intensity);
                spot->SetRange(descriptor.Range);
                spot->SetDirection(descriptor.Direction);
                spot->SetInnerCone(descriptor.InnerCone);
                spot->SetOuterCone(descriptor.OuterCone);

                light->pLight = spot;
                break;
            }
            case Light::LightType::DIRECTIONAL: {
                auto* directional = new DirectionalLight();
                directional->Create();

                directional->SetColor(descriptor.EmissionColor);
                directional->SetAmbient(descriptor.AmbientColor);
                directional->SetIntensity(descriptor.Intensity);
                directional->SetDirection(descriptor.Direction);

                light->pLight = directional;
                break;
            }
        }

        return pointer_cast<LightAsset*>(AddAsset(light));
    }

    ShaderAsset* Assets::AddShader(const std::string &name, const std::string &path, Shader::Type type) {
        if (m_Assets.contains(name)) return pointer_cast<ShaderAsset*>(m_Assets[name]);
        ShaderAsset* shader = new ShaderAsset();
        shader->Name = name;
        ErrorType err = shader->ShaderCode.Create(path, type);
        if (err != ErrorType::SUCCESS) {
            delete shader;
            return 0;
        }
        return pointer_cast<ShaderAsset*>(AddAsset(shader));
    }

    GraphicProgramAsset* Assets::AddGraphicProgram(const std::string &name, ShaderAsset *vertex, ShaderAsset *fragment, CullMode cull_mode, PolygonMode polygon_mode, const std::vector<VertexInputInformation> &vertex_input_info, const std::vector<DescriptorSetInformation> &descriptor_set_infos, RenderPassType type, RenderWindow *pWindow) {
        if (m_Assets.contains(name)) return pointer_cast<GraphicProgramAsset*>(m_Assets[name]);
        GraphicProgramAsset* graphic_program = new GraphicProgramAsset();
        graphic_program->Name = name;

        graphic_program->pVertex = vertex;
        graphic_program->pFragment = fragment;

        ProgramInformation pInformation;
        pInformation.DepthWriteEnable = true;
        pInformation.Parameters.CullMode_ = cull_mode;
        pInformation.Parameters.PolygoneMode = polygon_mode;
        switch (type) {
            case GRAPHICS_3D: pInformation.RenderPass_ = &pWindow->GetRenderTarget().GetRenderPass3D(); break;
            case GRAPHICS_2D: pInformation.RenderPass_ = &pWindow->GetRenderTarget().GetRenderPass2D(); break;
            case COMPUTE: pInformation.RenderPass_ = &pWindow->GetRenderTarget().GetRenderPass3D(); break;
        }

        pInformation.Shaders = {
            &fragment->ShaderCode,
            &vertex->ShaderCode
        };

        for (auto& descriptor_set : descriptor_set_infos) {
            graphic_program->Program.BindLayout( descriptor_set );
        }

        for (auto& vertex : vertex_input_info) {
            graphic_program->Program.BindVertexInput(vertex);
        }

        graphic_program->Program.Create(pInformation);

        return pointer_cast<GraphicProgramAsset*>(AddAsset(graphic_program));
    }

    ComputeProgramAsset* Assets::AddParticleProgram(const std::string &name, ParticleComputeInformation const& pInfo, ShaderAsset *compute)
    {
        if (m_Assets.contains(name)) return pointer_cast<ComputeProgramAsset*>(m_Assets[name]);
        ComputeProgramAsset* computeProgramAsset = new ComputeProgramAsset();
        computeProgramAsset->Name = name;
        computeProgramAsset->pCompute = compute;

        GPC::ComputeProgramInformation computeInformation {};
        computeInformation.Base.Shaders	= {&compute->ShaderCode};
        computeInformation.Type	= pInfo;

        computeProgramAsset->Program.Create(computeInformation);
        computeProgramAsset->Program.Update();

        return pointer_cast<ComputeProgramAsset*>(AddAsset(computeProgramAsset));
    }

    AudioResourceAsset* Assets::AddAudio(const std::string &name, const std::string &filePath)
    {
        if (m_Assets.contains(name)) return pointer_cast<AudioResourceAsset*>(m_Assets[name]);
        AudioResourceAsset* audioAsset = new AudioResourceAsset();
        audioAsset->Name = name;
        WavParser::LoadToMemory(filePath, *audioAsset);
        return pointer_cast<AudioResourceAsset*>(AddAsset(audioAsset));
    }


    MetaError Assets::LoadMeta(const std::string& metaPath)
    {
        if (!m_pMetaLoader)
            m_pMetaLoader = new MetaLoader(this);

        return m_pMetaLoader->Load(metaPath);
    }

    MetaError Assets::ResolveRuntimeAssets(const MetaRuntimeContext& ctx)
    {
        if (!m_pMetaLoader)
            return MetaError::SUCCESS;

        return m_pMetaLoader->ResolveRuntimeAssets(ctx);
    }

    bool Assets::HasPendingRuntimeAssets() const
    {
        return m_pMetaLoader && m_pMetaLoader->HasDeferredAssets();
    }

    TextureArrayAsset* Assets::AddTextureArray(const std::string &name, uint32_t width, uint32_t height, const std::vector<TextureAsset *> &pAssets) {
        if (m_Assets.contains(name)) return pointer_cast<TextureArrayAsset*>(m_Assets[name]);
        TextureArrayAsset* texture_array = new TextureArrayAsset();
        texture_array->Name = name;

        uint32_t i = 0;
        for (auto pAsset : pAssets) {
            auto texID = texture_array->Array.AddImage(pAsset->GetImageInformation(), i++);
            texture_array->PairAssetID[pAsset->Name] = { pAsset, texID };
        }

        TextureArrayInformation textureArrayInformation;
        textureArrayInformation.Width = width;
        textureArrayInformation.Height = height;

        texture_array->Array.Create(textureArrayInformation);

        constexpr SamplerInformation samplerInfo = {};
        texture_array->TextureSampler.Create(samplerInfo);

        return pointer_cast<TextureArrayAsset*>(AddAsset(texture_array));
    }

    FontAsset* Assets::AddFont(const std::string &name, const std::string &filePath, uint32_t fontSize)
    {
        if (m_Assets.contains(name)) return pointer_cast<FontAsset*>(m_Assets[name]);
        FontAsset* fontAsset = new FontAsset();
        fontAsset->Name = name;

        fontAsset->Font_.Create(filePath, fontSize);

        return pointer_cast<FontAsset*>(AddAsset(fontAsset));
    }

    void Assets::FreeAssets()
    {
        for (auto& it : m_Assets) {
            delete it.second;
        }
        m_Assets.clear();
    }


    /////// DELETED ////////
    // AssetID Assets::AddAnimation(const std::string& name, float duration)
    // {
    //     AnimationAsset* anim = new AnimationAsset();
    //     anim->Name = name;
    //     anim->TotalDuration = duration;
    //
    //     return AddAsset(anim);
    // }
    //
    // AssetID Assets::AddSprite(const std::string& name, const std::string& textureName, float uMin, float vMin, float uMax, float vMax)
    // {
    //     Asset* temp = GetAssets(textureName);
    //     if (!temp) return 0;
    //
    //     SpriteAsset* sprite = new SpriteAsset();
    //     sprite->Name = name;
    //     sprite->Texture = temp->ToTexture();
    //     sprite->UMin = uMin;
    //     sprite->VMin = vMin;
    //     sprite->UMax = uMax;
    //     sprite->VMax = vMax;
    //
    //     return AddAsset(sprite);
    // }
    //
    // AssetID Assets::AddSpriteSheet(const std::string& name, const std::string& textureName, uint32_t fWidth, uint32_t fHeight)
    // {
    //     Asset* base = GetAssets(textureName);
    //     if (!base) return 0;
    //     TextureAsset* tex = base->ToTexture();
    //
    //     SpriteSheetAsset* sheet = new SpriteSheetAsset();
    //     sheet->Name = name;
    //     sheet->Texture = tex;
    //     sheet->FrameWidth = fWidth;
    //     sheet->FrameHeight = fHeight;
    //
    //     uint32_t cols = tex->Width / fWidth;
    //     uint32_t rows = tex->Height / fHeight;
    //
    //     for (uint32_t y = 0; y < rows; y++) {
    //         for (uint32_t x = 0; x < cols; x++) {
    //             SpriteAsset* frame = new SpriteAsset();
    //
    //             frame->Name = name + "_" + std::to_string(y * cols + x);
    //             frame->Texture = tex;
    //             frame->Width = fWidth;
    //             frame->Height = fHeight;
    //
    //             frame->UMin = static_cast<float>(x * fWidth) / tex->Width;
    //             frame->VMin = static_cast<float>(y * fHeight) / tex->Height;
    //             frame->UMax = static_cast<float>((x + 1) * fWidth) / tex->Width;
    //             frame->VMax = static_cast<float>((y + 1) * fHeight) / tex->Height;
    //
    //             AddAsset(frame);
    //             sheet->Frames.push_back(frame);
    //         }
    //     }
    //     return AddAsset(sheet);
    // }

    // void Assets::LoadAtlas(const std::string& filePath)
    // {
    //     ReadStream stream(filePath.c_str());
    //     if (stream.GetSize() == 0) return;
    //
    //     // Serializer reader(&stream);
    //
    //     std::string textureName; // nom de l'atlas
    //     Asset* texAsset = this->GetAssets(textureName);
    //
    //     DEBUG_ASSERT(texAsset != nullptr,  "Erreur Atlas : La texture " << textureName << " n'est pas chargée.");
    //
    //     TextureAsset* pTex = texAsset->ToTexture();
    //
    //     uint32_t spriteCount; // nombre de sprite dans l'atlas
    //
    //     for (uint32_t i = 0; i < spriteCount; ++i)
    //     {
    //         SpriteAsset* sprite = new SpriteAsset();
    //
    //         sprite->Name;      // Nom du sprite
    //         uint32_t x;        // Position x en haut a gauche du sprite
    //         uint32_t y;        // position y en haut a gauche du sprite
    //         sprite->Width;     // largeur du sprite
    //         sprite->Height;    // hauteur du sprite
    //
    //         sprite->Texture = pTex;
    //
    //         sprite->UMin = (float)x / pTex->Width;
    //         sprite->VMin = (float)y / pTex->Height;
    //         sprite->UMax = (float)(x + sprite->Width) / pTex->Width;
    //         sprite->VMax = (float)(y + sprite->Height) / pTex->Height;
    //
    //         this->AddAsset(sprite);
    //     }
    // }

}
