//
// Created by killi on 25/03/2026.
//

#ifndef RUINS_OF_HESTIA_METALOADER_H
#define RUINS_OF_HESTIA_METALOADER_H

#include "MetaFile.h"
#include "MetaReader.h"
#include <unordered_set>

namespace GPC
{
    class Assets;

    class MetaLoader
    {
    public:
        explicit MetaLoader(Assets* pAssets);
        ~MetaLoader() = default;

        MetaLoader(const MetaLoader&) = delete;
        MetaLoader& operator=(const MetaLoader&) = delete;
        MetaLoader(MetaLoader&&) noexcept = default;
        MetaLoader& operator=(MetaLoader&&) noexcept = default;

        MetaError Load(const std::string& metaPath);
        MetaError ResolveRuntimeAssets(const MetaRuntimeContext& ctx);

        [[nodiscard]] uint32_t GetLoadedCount() const { return m_LoadedCount; }
        [[nodiscard]] bool HasDeferredAssets() const { return !m_DeferredAssets.empty(); }
        [[nodiscard]] const std::vector<MetaDeferredAsset>& GetDeferredAssets() const { return m_DeferredAssets; }

    private:
        MetaError LoadAsset(const MetaAsset& asset, const std::string& basePath);

        MetaError LoadTexture(const MetaAsset& asset, const std::string& basePath);
        [[nodiscard]] MetaError LoadMesh(const MetaAsset& asset) const;
        MetaError LoadShader(const MetaAsset& asset, const std::string& basePath);
        [[nodiscard]] MetaError LoadLight(const MetaAsset& asset) const;
        MetaError LoadAudio(const MetaAsset& asset, const std::string& basePath);
        // [[nodiscard]] MetaError LoadSprite(const MetaAsset& asset) const; // TODO DISABLE (NOT USED IN PROJECT)
        // [[nodiscard]] MetaError LoadSpriteSheet(const MetaAsset& asset) const;
        // [[nodiscard]] MetaError LoadAnimation(const MetaAsset& asset) const;

        void DeferAsset(const MetaAsset& asset);
        MetaError ResolveGraphicProgram(const MetaDeferredAsset& deferred, const MetaRuntimeContext& ctx);
        MetaError ResolveComputeProgram(const MetaDeferredAsset& deferred, const MetaRuntimeContext& ctx);
        MetaError ResolveTextureArray(const MetaDeferredAsset& deferred, const MetaRuntimeContext& ctx);

        MetaError LoadInclude(const MetaAsset& asset, const std::string& currentMetaPath);
        std::string ResolvePath(const std::string& basePath, const std::string& assetPath);
        std::string NormalizePath(const std::string& path);

        Assets* m_pAssets;
        uint32_t m_LoadedCount = 0;
        std::vector<MetaDeferredAsset> m_DeferredAssets;
        std::unordered_set<std::string> m_LoadingStack;
    };

} // GPC

#endif //RUINS_OF_HESTIA_METALOADER_H