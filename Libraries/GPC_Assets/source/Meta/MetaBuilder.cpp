//
// Created by killi on 27/03/2026.
//

#include "Meta/MetaBuilder.h"
#include "Meta/MetaWriter.h"
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace GPC
{

MetaBuilder MetaBuilder::Create()
{
    return MetaBuilder();
}

MetaBuilder::MetaBuilder()
{
    m_File.Magic = META_MAGIC;
    m_File.Version = META_VERSION;
}

MetaBuilder& MetaBuilder::EnableDuplicateWarning(bool enable)
{
    m_WarnDuplicates = enable;
    return *this;
}

void MetaBuilder::Add(MetaAssetType type, const std::string& name, const std::string& path)
{
    MetaAsset asset;
    asset.Type = type;
    asset.Name = name;
    asset.Path = path;
    AddAsset(std::move(asset));
}

void MetaBuilder::AddAsset(MetaAsset&& asset)
{
    if (m_WarnDuplicates && HasAsset(asset.Name, asset.Type))
        WarnDuplicate(asset.Name, asset.Type);
    m_File.Assets.push_back(std::move(asset));
}

bool MetaBuilder::HasAsset(const std::string& name, MetaAssetType type) const
{
    for (const auto& a : m_File.Assets)
    {
        if (a.Name == name && a.Type == type)
            return true;
    }
    return false;
}

void MetaBuilder::WarnDuplicate(const std::string& name, MetaAssetType type)
{
    std::cerr << "[MetaBuilder] Warning: Duplicate asset '" << name
              << "' (type " << MetaAssetTypeToString(type) << ")\n";
}

// === Core Assets ===

MetaBuilder& MetaBuilder::Shader(const std::string& name, const std::string& path)
{
    Add(MetaAssetType::SHADER, name, path);
    return *this;
}

MetaBuilder& MetaBuilder::Texture(const std::string& name, const std::string& path)
{
    Add(MetaAssetType::TEXTURE, name, path);
    return *this;
}

MetaBuilder& MetaBuilder::Mesh(const std::string& name, const std::string& primitive)
{
    Add(MetaAssetType::MESH, name, "primitive:" + primitive);
    return *this;
}

MetaBuilder& MetaBuilder::MeshFile(const std::string& name, const std::string& path)
{
    Add(MetaAssetType::MESH, name, path);
    return *this;
}

MetaBuilder& MetaBuilder::Audio(const std::string& name, const std::string& path)
{
    Add(MetaAssetType::AUDIO, name, path);
    return *this;
}

// === Sous-Builders ===

LightBuilder MetaBuilder::Light(const std::string& name)
{
    return LightBuilder(*this, name);
}

GraphicProgramBuilder MetaBuilder::GraphicProgram(const std::string& name)
{
    return GraphicProgramBuilder(*this, name);
}

TextureArrayBuilder MetaBuilder::TextureArray(const std::string& name)
{
    return TextureArrayBuilder(*this, name);
}

SpriteBuilder MetaBuilder::Sprite(const std::string& name)
{
    return SpriteBuilder(*this, name);
}

SpriteSheetBuilder MetaBuilder::SpriteSheet(const std::string& name)
{
    return SpriteSheetBuilder(*this, name);
}

AnimationBuilder MetaBuilder::Animation(const std::string& name)
{
    return AnimationBuilder(*this, name);
}

// === Rétrocompat ===

MetaBuilder& MetaBuilder::LightLegacy(const std::string& name, const std::string& config)
{
    Add(MetaAssetType::LIGHT, name, config);
    return *this;
}

MetaBuilder& MetaBuilder::GraphicProgramLegacy(const std::string& name,
                                                const std::string& vert, const std::string& frag)
{
    Add(MetaAssetType::GRAPHIC_PROGRAM, name, vert + "," + frag);
    return *this;
}

MetaBuilder& MetaBuilder::TextureArrayLegacy(const std::string& name, const std::string& textures,
                                              uint32_t width, uint32_t height)
{
    std::ostringstream ss;
    ss << textures << ":" << width << "," << height;
    Add(MetaAssetType::TEXTURE_ARRAY, name, ss.str());
    return *this;
}

MetaBuilder& MetaBuilder::ComputeProgram(const std::string& name, const std::string& computeShader)
{
    Add(MetaAssetType::COMPUTE_PROGRAM, name, computeShader);
    return *this;
}

// === Composition ===

MetaBuilder& MetaBuilder::Include(const std::string& metaPath)
{
    Add(MetaAssetType::META_INCLUDE, "", metaPath);
    return *this;
}

// === Output ===

MetaFile MetaBuilder::Build() const
{
    MetaFile copy;
    copy.Magic = m_File.Magic;
    copy.Version = m_File.Version;
    copy.SourcePath = m_File.SourcePath;

    for (const auto& asset : m_File.Assets)
    {
        MetaAsset newAsset;
        newAsset.Type = asset.Type;
        newAsset.Name = asset.Name;
        newAsset.Path = asset.Path;
        newAsset.Optional = asset.Optional;
        newAsset.Loaded = asset.Loaded;
        if (asset.Properties)
        {
            newAsset.Properties = std::make_unique<SerialTree>();
        }
        copy.Assets.push_back(std::move(newAsset));
    }

    return copy;
}

bool MetaBuilder::WriteTo(const std::string& path) const
{
    fs::path filePath(path);
    if (filePath.has_parent_path())
        fs::create_directories(filePath.parent_path());

    MetaWriter writer;
    return writer.Write(m_File, path) == MetaError::SUCCESS;
}

// ============================================================================
// LightBuilder
// ============================================================================

LightBuilder::LightBuilder(MetaBuilder& parent, const std::string& name)
    : m_Parent(parent), m_Name(name)
{
}

LightBuilder& LightBuilder::Type(Light::LightType type)
{
    m_Type = type;
    return *this;
}

LightBuilder& LightBuilder::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    m_R = r; m_G = g; m_B = b; m_A = a;
    return *this;
}

LightBuilder& LightBuilder::Intensity(float intensity)
{
    m_Intensity = intensity;
    return *this;
}

LightBuilder& LightBuilder::Range(float range)
{
    m_Range = range;
    return *this;
}

LightBuilder& LightBuilder::WithProperty(const std::string& key, const std::string& value)
{
    m_CustomProps.emplace_back(key, value);
    return *this;
}

MetaBuilder& LightBuilder::Build()
{
    MetaAsset asset;
    asset.Type = MetaAssetType::LIGHT;
    asset.Name = m_Name;
    asset.Path = "";

    asset.Properties = std::make_unique<SerialTree>();
    SerialTree& props = *asset.Properties;

    std::string typeStr;
    switch (m_Type)
    {
        case Light::LightType::POINT:       typeStr = "point"; break;
        case Light::LightType::SPOT:        typeStr = "spot"; break;
        case Light::LightType::DIRECTIONAL: typeStr = "directional"; break;
    }
    props["type"].Set(const_cast<char*>(typeStr.data()), typeStr.size(), true);

    props["r"].Set<uint8_t>(&m_R, 1);
    props["g"].Set<uint8_t>(&m_G, 1);
    props["b"].Set<uint8_t>(&m_B, 1);
    props["a"].Set<uint8_t>(&m_A, 1);
    props["intensity"].Set<float>(&m_Intensity, 1);
    props["range"].Set<float>(&m_Range, 1);

    for (const auto& [key, value] : m_CustomProps)
        props[key].Set(const_cast<char*>(value.data()), value.size(), true);

    m_Parent.AddAsset(std::move(asset));
    return m_Parent;
}

// ============================================================================
// GraphicProgramBuilder
// ============================================================================

GraphicProgramBuilder::GraphicProgramBuilder(MetaBuilder& parent, const std::string& name)
    : m_Parent(parent), m_Name(name)
{
}

GraphicProgramBuilder& GraphicProgramBuilder::Vertex(const std::string& shaderName)
{
    m_VertexShader = shaderName;
    return *this;
}

GraphicProgramBuilder& GraphicProgramBuilder::Fragment(const std::string& shaderName)
{
    m_FragmentShader = shaderName;
    return *this;
}

GraphicProgramBuilder& GraphicProgramBuilder::CullMode(GPC::CullMode mode)
{
    m_CullMode = mode;
    return *this;
}

GraphicProgramBuilder& GraphicProgramBuilder::PolygonMode(GPC::PolygonMode mode)
{
    m_PolygonMode = mode;
    return *this;
}

GraphicProgramBuilder& GraphicProgramBuilder::WithProperty(const std::string& key, const std::string& value)
{
    m_CustomProps.emplace_back(key, value);
    return *this;
}

MetaBuilder& GraphicProgramBuilder::Build()
{
    MetaAsset asset;
    asset.Type = MetaAssetType::GRAPHIC_PROGRAM;
    asset.Name = m_Name;
    asset.Path = m_VertexShader + "," + m_FragmentShader;

    asset.Properties = std::make_unique<SerialTree>();
    SerialTree& props = *asset.Properties;

    props["vertex"].Set(const_cast<char*>(m_VertexShader.data()), m_VertexShader.size(), true);
    props["fragment"].Set(const_cast<char*>(m_FragmentShader.data()), m_FragmentShader.size(), true);

    uint8_t cullVal = static_cast<uint8_t>(m_CullMode);
    uint8_t polyVal = static_cast<uint8_t>(m_PolygonMode);
    props["cullMode"].Set<uint8_t>(&cullVal, 1);
    props["polygonMode"].Set<uint8_t>(&polyVal, 1);

    for (const auto& [key, value] : m_CustomProps)
        props[key].Set(const_cast<char*>(value.data()), value.size(), true);

    m_Parent.AddAsset(std::move(asset));
    return m_Parent;
}

// ============================================================================
// TextureArrayBuilder
// ============================================================================

TextureArrayBuilder::TextureArrayBuilder(MetaBuilder& parent, const std::string& name)
    : m_Parent(parent), m_Name(name)
{
}

TextureArrayBuilder& TextureArrayBuilder::Textures(const std::vector<std::string>& textures)
{
    m_Textures = textures;
    return *this;
}

TextureArrayBuilder& TextureArrayBuilder::AddTexture(const std::string& textureName)
{
    m_Textures.push_back(textureName);
    return *this;
}

TextureArrayBuilder& TextureArrayBuilder::Size(uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;
    return *this;
}

TextureArrayBuilder& TextureArrayBuilder::WithProperty(const std::string& key, const std::string& value)
{
    m_CustomProps.emplace_back(key, value);
    return *this;
}

MetaBuilder& TextureArrayBuilder::Build()
{
    MetaAsset asset;
    asset.Type = MetaAssetType::TEXTURE_ARRAY;
    asset.Name = m_Name;

    std::ostringstream pathStream;
    for (size_t i = 0; i < m_Textures.size(); ++i)
    {
        if (i > 0) pathStream << ",";
        pathStream << m_Textures[i];
    }
    pathStream << ":" << m_Width << "," << m_Height;
    asset.Path = pathStream.str();

    asset.Properties = std::make_unique<SerialTree>();
    SerialTree& props = *asset.Properties;

    std::ostringstream texStream;
    for (size_t i = 0; i < m_Textures.size(); ++i)
    {
        if (i > 0) texStream << ",";
        texStream << m_Textures[i];
    }
    std::string texList = texStream.str();
    props["textures"].Set(const_cast<char*>(texList.data()), texList.size(), true);
    props["width"].Set<uint32_t>(&m_Width, 1);
    props["height"].Set<uint32_t>(&m_Height, 1);

    for (const auto& [key, value] : m_CustomProps)
        props[key].Set(const_cast<char*>(value.data()), value.size(), true);

    m_Parent.AddAsset(std::move(asset));
    return m_Parent;
}

// ============================================================================
// SpriteBuilder
// ============================================================================

SpriteBuilder::SpriteBuilder(MetaBuilder& parent, const std::string& name)
    : m_Parent(parent), m_Name(name)
{
}

SpriteBuilder& SpriteBuilder::Texture(const std::string& textureName)
{
    m_Texture = textureName;
    return *this;
}

SpriteBuilder& SpriteBuilder::Region(int x, int y, int w, int h)
{
    m_X = x; m_Y = y; m_W = w; m_H = h;
    return *this;
}

SpriteBuilder& SpriteBuilder::WithProperty(const std::string& key, const std::string& value)
{
    m_CustomProps.emplace_back(key, value);
    return *this;
}

MetaBuilder& SpriteBuilder::Build()
{
    MetaAsset asset;
    asset.Type = MetaAssetType::SPRITE;
    asset.Name = m_Name;

    std::ostringstream ss;
    ss << m_Texture << ":" << m_X << "," << m_Y << "," << m_W << "," << m_H;
    asset.Path = ss.str();

    asset.Properties = std::make_unique<SerialTree>();
    SerialTree& props = *asset.Properties;

    props["texture"].Set(const_cast<char*>(m_Texture.data()), m_Texture.size(), true);
    props["x"].Set<int>(&m_X, 1);
    props["y"].Set<int>(&m_Y, 1);
    props["w"].Set<int>(&m_W, 1);
    props["h"].Set<int>(&m_H, 1);

    for (const auto& [key, value] : m_CustomProps)
        props[key].Set(const_cast<char*>(value.data()), value.size(), true);

    m_Parent.AddAsset(std::move(asset));
    return m_Parent;
}

// ============================================================================
// SpriteSheetBuilder
// ============================================================================

SpriteSheetBuilder::SpriteSheetBuilder(MetaBuilder& parent, const std::string& name)
    : m_Parent(parent), m_Name(name)
{
}

SpriteSheetBuilder& SpriteSheetBuilder::Texture(const std::string& textureName)
{
    m_Texture = textureName;
    return *this;
}

SpriteSheetBuilder& SpriteSheetBuilder::Grid(int cols, int rows)
{
    m_Cols = cols;
    m_Rows = rows;
    return *this;
}

SpriteSheetBuilder& SpriteSheetBuilder::FrameSize(int frameW, int frameH)
{
    m_FrameW = frameW;
    m_FrameH = frameH;
    return *this;
}

SpriteSheetBuilder& SpriteSheetBuilder::WithProperty(const std::string& key, const std::string& value)
{
    m_CustomProps.emplace_back(key, value);
    return *this;
}

MetaBuilder& SpriteSheetBuilder::Build()
{
    MetaAsset asset;
    asset.Type = MetaAssetType::SPRITE_SHEET;
    asset.Name = m_Name;

    std::ostringstream ss;
    ss << m_Texture << ":" << m_FrameW << "," << m_FrameH;
    asset.Path = ss.str();

    asset.Properties = std::make_unique<SerialTree>();
    SerialTree& props = *asset.Properties;

    props["texture"].Set(const_cast<char*>(m_Texture.data()), m_Texture.size(), true);
    props["cols"].Set<int>(&m_Cols, 1);
    props["rows"].Set<int>(&m_Rows, 1);
    props["frameW"].Set<int>(&m_FrameW, 1);
    props["frameH"].Set<int>(&m_FrameH, 1);

    for (const auto& [key, value] : m_CustomProps)
        props[key].Set(const_cast<char*>(value.data()), value.size(), true);

    m_Parent.AddAsset(std::move(asset));
    return m_Parent;
}

// ============================================================================
// AnimationBuilder
// ============================================================================

AnimationBuilder::AnimationBuilder(MetaBuilder& parent, const std::string& name)
    : m_Parent(parent), m_Name(name)
{
}

AnimationBuilder& AnimationBuilder::SpriteSheet(const std::string& sheetName)
{
    m_SpriteSheet = sheetName;
    return *this;
}

AnimationBuilder& AnimationBuilder::Sprites(const std::vector<std::string>& spriteNames)
{
    m_Sprites = spriteNames;
    return *this;
}

AnimationBuilder& AnimationBuilder::Duration(float duration)
{
    m_Duration = duration;
    return *this;
}

AnimationBuilder& AnimationBuilder::WithProperty(const std::string& key, const std::string& value)
{
    m_CustomProps.emplace_back(key, value);
    return *this;
}

MetaBuilder& AnimationBuilder::Build()
{
    MetaAsset asset;
    asset.Type = MetaAssetType::ANIMATION;
    asset.Name = m_Name;

    std::ostringstream ss;
    ss << m_SpriteSheet << ":" << m_Duration;
    asset.Path = ss.str();

    asset.Properties = std::make_unique<SerialTree>();
    SerialTree& props = *asset.Properties;

    props["spriteSheet"].Set(const_cast<char*>(m_SpriteSheet.data()), m_SpriteSheet.size(), true);
    props["duration"].Set<float>(&m_Duration, 1);

    if (!m_Sprites.empty())
    {
        std::ostringstream spritesStream;
        for (size_t i = 0; i < m_Sprites.size(); ++i)
        {
            if (i > 0) spritesStream << ",";
            spritesStream << m_Sprites[i];
        }
        std::string spriteList = spritesStream.str();
        props["sprites"].Set(const_cast<char*>(spriteList.data()), spriteList.size(), true);
    }

    for (const auto& [key, value] : m_CustomProps)
        props[key].Set(const_cast<char*>(value.data()), value.size(), true);

    m_Parent.AddAsset(std::move(asset));
    return m_Parent;
}

} // namespace GPC
