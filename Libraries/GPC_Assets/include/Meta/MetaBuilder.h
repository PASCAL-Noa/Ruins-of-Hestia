//
// Created by killi on 27/03/2026.
//

#ifndef RUINS_OF_HESTIA_METABUILDER_H
#define RUINS_OF_HESTIA_METABUILDER_H

#include "Meta/MetaFile.h"
#include "Meta/MetaTypes.h"
#include "Light.h"
#include "IProgram.h"
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>

namespace GPC
{

class MetaBuilder;
class LightBuilder;
class GraphicProgramBuilder;
class TextureArrayBuilder;
class SpriteBuilder;
class SpriteSheetBuilder;
class AnimationBuilder;

class MetaBuilder
{
public:
    static MetaBuilder Create();

    MetaBuilder& EnableDuplicateWarning(bool enable = true);

    // === Core Assets (inchangés) ===
    MetaBuilder& Shader(const std::string& name, const std::string& path);
    MetaBuilder& Texture(const std::string& name, const std::string& path);
    MetaBuilder& Mesh(const std::string& name, const std::string& primitive);
    MetaBuilder& MeshFile(const std::string& name, const std::string& path);
    MetaBuilder& Audio(const std::string& name, const std::string& path);

    // === Sous-Builders ===
    LightBuilder Light(const std::string& name);
    GraphicProgramBuilder GraphicProgram(const std::string& name);
    TextureArrayBuilder TextureArray(const std::string& name);
    SpriteBuilder Sprite(const std::string& name);
    SpriteSheetBuilder SpriteSheet(const std::string& name);
    AnimationBuilder Animation(const std::string& name);

    // === Rétrocompat (déprécié) ===
    [[deprecated("Use Light(name).Type().Color().Build()")]]
    MetaBuilder& LightLegacy(const std::string& name, const std::string& config);
    [[deprecated("Use GraphicProgram(name).Vertex().Fragment().Build()")]]
    MetaBuilder& GraphicProgramLegacy(const std::string& name,
                                       const std::string& vert, const std::string& frag);
    [[deprecated("Use TextureArray(name).Textures().Size().Build()")]]
    MetaBuilder& TextureArrayLegacy(const std::string& name, const std::string& textures,
                                     uint32_t width, uint32_t height);

    MetaBuilder& ComputeProgram(const std::string& name, const std::string& computeShader);

    // === Composition ===
    MetaBuilder& Include(const std::string& metaPath);

    // === Output ===
    [[nodiscard]] MetaFile Build() const;
    bool WriteTo(const std::string& path) const;

    // === Internal (pour sous-builders) ===
    void AddAsset(MetaAsset&& asset);
    bool HasAsset(const std::string& name, MetaAssetType type) const;

private:
    MetaBuilder();
    void WarnDuplicate(const std::string& name, MetaAssetType type);
    void Add(MetaAssetType type, const std::string& name, const std::string& path);

    MetaFile m_File;
    bool m_WarnDuplicates = true;
};

class LightBuilder
{
public:
    LightBuilder(MetaBuilder& parent, const std::string& name);

    LightBuilder& Type(Light::LightType type);
    LightBuilder& Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    LightBuilder& Intensity(float intensity);
    LightBuilder& Range(float range);
    LightBuilder& WithProperty(const std::string& key, const std::string& value);

    MetaBuilder& Build();

private:
    MetaBuilder& m_Parent;
    std::string m_Name;
    Light::LightType m_Type = Light::LightType::POINT;
    uint8_t m_R = 255, m_G = 255, m_B = 255, m_A = 255;
    float m_Intensity = 1.0f;
    float m_Range = 5.0f;
    std::vector<std::pair<std::string, std::string>> m_CustomProps;
};

class GraphicProgramBuilder
{
public:
    GraphicProgramBuilder(MetaBuilder& parent, const std::string& name);

    GraphicProgramBuilder& Vertex(const std::string& shaderName);
    GraphicProgramBuilder& Fragment(const std::string& shaderName);
    GraphicProgramBuilder& CullMode(GPC::CullMode mode);
    GraphicProgramBuilder& PolygonMode(GPC::PolygonMode mode);
    GraphicProgramBuilder& WithProperty(const std::string& key, const std::string& value);

    MetaBuilder& Build();

private:
    MetaBuilder& m_Parent;
    std::string m_Name;
    std::string m_VertexShader;
    std::string m_FragmentShader;
    GPC::CullMode m_CullMode = GPC::CullMode::FRONT;
    GPC::PolygonMode m_PolygonMode = GPC::PolygonMode::MODE_FILL;
    std::vector<std::pair<std::string, std::string>> m_CustomProps;
};

class TextureArrayBuilder
{
public:
    TextureArrayBuilder(MetaBuilder& parent, const std::string& name);

    TextureArrayBuilder& Textures(const std::vector<std::string>& textures);
    TextureArrayBuilder& AddTexture(const std::string& textureName);
    TextureArrayBuilder& Size(uint32_t width, uint32_t height);
    TextureArrayBuilder& WithProperty(const std::string& key, const std::string& value);

    MetaBuilder& Build();

private:
    MetaBuilder& m_Parent;
    std::string m_Name;
    std::vector<std::string> m_Textures;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    std::vector<std::pair<std::string, std::string>> m_CustomProps;
};

class SpriteBuilder
{
public:
    SpriteBuilder(MetaBuilder& parent, const std::string& name);

    SpriteBuilder& Texture(const std::string& textureName);
    SpriteBuilder& Region(int x, int y, int w, int h);
    SpriteBuilder& WithProperty(const std::string& key, const std::string& value);

    MetaBuilder& Build();

private:
    MetaBuilder& m_Parent;
    std::string m_Name;
    std::string m_Texture;
    int m_X = 0, m_Y = 0, m_W = 0, m_H = 0;
    std::vector<std::pair<std::string, std::string>> m_CustomProps;
};

class SpriteSheetBuilder
{
public:
    SpriteSheetBuilder(MetaBuilder& parent, const std::string& name);

    SpriteSheetBuilder& Texture(const std::string& textureName);
    SpriteSheetBuilder& Grid(int cols, int rows);
    SpriteSheetBuilder& FrameSize(int frameW, int frameH);
    SpriteSheetBuilder& WithProperty(const std::string& key, const std::string& value);

    MetaBuilder& Build();

private:
    MetaBuilder& m_Parent;
    std::string m_Name;
    std::string m_Texture;
    int m_Cols = 1, m_Rows = 1;
    int m_FrameW = 0, m_FrameH = 0;
    std::vector<std::pair<std::string, std::string>> m_CustomProps;
};

class AnimationBuilder
{
public:
    AnimationBuilder(MetaBuilder& parent, const std::string& name);

    AnimationBuilder& SpriteSheet(const std::string& sheetName);
    AnimationBuilder& Sprites(const std::vector<std::string>& spriteNames);
    AnimationBuilder& Duration(float duration);
    AnimationBuilder& WithProperty(const std::string& key, const std::string& value);

    MetaBuilder& Build();

private:
    MetaBuilder& m_Parent;
    std::string m_Name;
    std::string m_SpriteSheet;
    std::vector<std::string> m_Sprites;
    float m_Duration = 1.0f;
    std::vector<std::pair<std::string, std::string>> m_CustomProps;
};

} // namespace GPC

#endif //RUINS_OF_HESTIA_METABUILDER_H
