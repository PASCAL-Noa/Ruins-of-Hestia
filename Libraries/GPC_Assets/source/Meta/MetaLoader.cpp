//
// Created by killi on 25/03/2026.
//

#include "Meta/MetaLoader.h"
#include "Assets.h"
#include "Primitive.h"
#include "WavParser.h"

#include <cstdio>
#include <filesystem>
#include <sstream>

namespace GPC
{

  namespace fs = std::filesystem;

  MetaLoader::MetaLoader(Assets* pAssets)
      : m_pAssets(pAssets)
      , m_LoadedCount(0)
  {
  }

  std::string MetaLoader::NormalizePath(const std::string& path)
  {
      return fs::absolute(path).lexically_normal().string();
  }

  MetaError MetaLoader::Load(const std::string& metaPath)
  {
      if (!m_pAssets)
          return MetaError::ASSET_LOAD_FAILED;

      std::string normalizedPath = NormalizePath(metaPath);

      if (m_LoadingStack.contains(normalizedPath))
      {
          GPC_ERROR << "MetaLoader: Cycle detected! '" << metaPath << "' is already being loaded" << ENDL;
          return MetaError::CYCLE_DETECTED;
      }

      m_LoadingStack.insert(normalizedPath);

      MetaFile file;
      MetaReader reader;
      MetaError err = reader.Read(metaPath, file);
      if (err != MetaError::SUCCESS)
      {
          m_LoadingStack.erase(normalizedPath);
          return err;
      }

      bool isRootLoad = (m_LoadingStack.size() == 1);
      if (isRootLoad)
          m_LoadedCount = 0;

      for (const auto& asset : file.Assets)
      {
          if (asset.Type == MetaAssetType::META_INCLUDE)
          {
              err = LoadInclude(asset, metaPath);
          }
          else
          {
              err = LoadAsset(asset, "");
          }

          if (err != MetaError::SUCCESS)
          {
              if (!asset.Optional)
              {
                  GPC_ERROR << "MetaLoader: Failed to load required asset '" << asset.Name << "'" << ENDL;
                  m_LoadingStack.erase(normalizedPath);
                  return err;
              }
              GPC_INFO << "MetaLoader: Skipped optional asset '" << asset.Name << "'" << ENDL;
          }
          else
          {
              ++m_LoadedCount;
          }
      }

      m_LoadingStack.erase(normalizedPath);

      if (isRootLoad)
          GPC_INFO << "MetaLoader: Loaded " << m_LoadedCount << " assets from " << metaPath << ENDL;

      return MetaError::SUCCESS;
  }

  MetaError MetaLoader::LoadAsset(const MetaAsset& asset, const std::string& basePath)
  {
      if (m_pAssets->HasAsset(asset.Name))
      {
          GPC_INFO << "MetaLoader: Skipping existing asset '" << asset.Name << "'" << ENDL;
          return MetaError::SUCCESS;
      }

      switch (asset.Type)
      {
          case MetaAssetType::TEXTURE:
              return LoadTexture(asset, basePath);
          case MetaAssetType::MESH:
              return LoadMesh(asset);
          case MetaAssetType::SHADER:
              return LoadShader(asset, basePath);
          case MetaAssetType::LIGHT:
              return LoadLight(asset);
          case MetaAssetType::AUDIO:
              return LoadAudio(asset, basePath);
          case MetaAssetType::GRAPHIC_PROGRAM:
          case MetaAssetType::COMPUTE_PROGRAM:
          case MetaAssetType::TEXTURE_ARRAY:
              DeferAsset(asset);
              return MetaError::SUCCESS;
          default:
              GPC_ERROR << "MetaLoader: Unknown asset type for '" << asset.Name << "'" << ENDL;
              return MetaError::UNKNOWN_ASSET_TYPE;
      }
  }

  std::string MetaLoader::ResolvePath(const std::string& basePath, const std::string& assetPath)
  {
      if (assetPath.empty())
          return "";

      fs::path path(assetPath);
      if (path.is_absolute())
          return assetPath;

      return (fs::path(basePath) / path).lexically_normal().string();
  }

  MetaError MetaLoader::LoadTexture(const MetaAsset& asset, const std::string& basePath)
  {
      std::string fullPath = ResolvePath(basePath, asset.Path);

      if (m_pAssets->AddTexture(asset.Name, fullPath) == nullptr)
      {
          GPC_ERROR << "MetaLoader: Texture load failed '" << fullPath << "'" << ENDL;
          return MetaError::ASSET_LOAD_FAILED;
      }

      return MetaError::SUCCESS;
  }

  MetaError MetaLoader::LoadMesh(const MetaAsset& asset) const
  {
      if (asset.Path.rfind("primitive:", 0) == 0)
      {
          std::string primType = asset.Path.substr(10);

          GeometryData data;

          if (primType == "cube")
              Primitive::CreateCube(glm::vec3(1.0f), &data);
          else if (primType == "sphere")
              Primitive::CreateSphere(0.5f, 32, 32, &data);
          else if (primType == "plane")
              Primitive::CreatePlane(glm::vec2(1.0f), 1, &data);
          else if (primType == "rectangle")
              Primitive::CreateRectangle(glm::vec2(1.0f), &data);
          else if (primType == "triangle")
              Primitive::CreateTristan(glm::vec2(1.0f), &data);
          else
          {
              GPC_ERROR << "MetaLoader: Unknown primitive '" << primType << "'" << ENDL;
              return MetaError::ASSET_LOAD_FAILED;
          }

          m_pAssets->AddMesh(asset.Name, &data);
          return MetaError::SUCCESS;
      }

      GPC_ERROR << "MetaLoader: Mesh file loading not implemented" << ENDL;
      return MetaError::ASSET_LOAD_FAILED;
  }

  MetaError MetaLoader::LoadShader(const MetaAsset& asset, const std::string& basePath)
  {
      std::string fullPath = ResolvePath(basePath, asset.Path);

      Shader::Type shaderType = Shader::Type::VERTEX;
      std::string ext = fs::path(fullPath).extension().string();

      if (ext == ".vert" || ext == ".vs")
          shaderType = Shader::Type::VERTEX;
      else if (ext == ".frag" || ext == ".fs")
          shaderType = Shader::Type::FRAGMENT;
      else if (ext == ".comp")
          shaderType = Shader::Type::COMPUTE;

      if (m_pAssets->AddShader(asset.Name, fullPath, shaderType) == nullptr)
      {
          GPC_ERROR << "MetaLoader: Shader load failed '" << fullPath << "'" << ENDL;
          return MetaError::ASSET_LOAD_FAILED;
      }

      return MetaError::SUCCESS;
  }

  MetaError MetaLoader::LoadLight(const MetaAsset& asset) const
  {
      LightDescriptor desc{};
      uint8_t r = 255, g = 255, b = 255, a = 255;
      float intensity = 1.0f;
      float range = 5.0f;

      if (asset.Properties && asset.Properties->ChildCount() > 0)
      {
          const SerialTree& props = *asset.Properties;

          if (props.HasKey("type"))
          {
              std::string typeStr(props["type"].Get<char>(), props["type"].ByteSize());
              if (typeStr == "spot")
                  desc.Type = Light::LightType::SPOT;
              else if (typeStr == "directional")
                  desc.Type = Light::LightType::DIRECTIONAL;
              else
                  desc.Type = Light::LightType::POINT;
          }

          if (props.HasKey("r"))
              r = *props["r"].Get<uint8_t>();
          if (props.HasKey("g"))
              g = *props["g"].Get<uint8_t>();
          if (props.HasKey("b"))
              b = *props["b"].Get<uint8_t>();
          if (props.HasKey("a"))
              a = *props["a"].Get<uint8_t>();
          if (props.HasKey("intensity"))
              intensity = *props["intensity"].Get<float>();
          if (props.HasKey("range"))
              range = *props["range"].Get<float>();
      }
      else
      {
          std::string typePart = asset.Path;
          size_t colonPos = asset.Path.find(':');

          if (colonPos != std::string::npos)
          {
              typePart = asset.Path.substr(0, colonPos);
              std::string colorPart = asset.Path.substr(colonPos + 1);

              uint32_t pr, pg, pb, pa;
              if (sscanf(colorPart.c_str(), "%u,%u,%u,%u", &pr, &pg, &pb, &pa) == 4)
              {
                  r = static_cast<uint8_t>(pr);
                  g = static_cast<uint8_t>(pg);
                  b = static_cast<uint8_t>(pb);
                  a = static_cast<uint8_t>(pa);
              }
          }

          if (typePart == "spot")
              desc.Type = Light::LightType::SPOT;
          else if (typePart == "directional")
              desc.Type = Light::LightType::DIRECTIONAL;
          else
              desc.Type = Light::LightType::POINT;
      }

      Color color{r, g, b, a};

      if (m_pAssets->AddLight(asset.Name, desc) != nullptr)
      {
          Asset* assetPtr = m_pAssets->GetAssets(asset.Name);
          if (assetPtr)
          {
              Light* light = assetPtr->ToLight()->pLight;
              light->SetColor(color);
              light->SetIntensity(intensity);
              light->SetRange(range);
          }
      }

      return MetaError::SUCCESS;
  }

  MetaError MetaLoader::LoadAudio(const MetaAsset& asset, const std::string& basePath)
  {
      std::string fullPath = ResolvePath(basePath, asset.Path);

      if (m_pAssets->AddAudio(asset.Name, fullPath) == nullptr)
      {
          GPC_ERROR << "MetaLoader: Audio load failed '" << fullPath << "'" << ENDL;
          return MetaError::ASSET_LOAD_FAILED;
      }

      return MetaError::SUCCESS;
  }

  // MetaError MetaLoader::LoadSprite(const MetaAsset& asset) const
  // {
  //     size_t colonPos = asset.Path.find(':');
  //     if (colonPos == std::string::npos)
  //     {
  //         GPC_ERROR << "MetaLoader: Invalid sprite format '" << asset.Path << "'" << ENDL;
  //         return MetaError::PARSE_ERROR;
  //     }
  //
  //     std::string textureName = asset.Path.substr(0, colonPos);
  //     std::string coords = asset.Path.substr(colonPos + 1);
  //
  //     float uMin, vMin, uMax, vMax;
  //     if (sscanf(coords.c_str(), "%f,%f,%f,%f", &uMin, &vMin, &uMax, &vMax) != 4)
  //     {
  //         GPC_ERROR << "MetaLoader: Invalid sprite coords '" << coords << "'" << ENDL;
  //         return MetaError::PARSE_ERROR;
  //     }
  //
  //     AssetID id = m_pAssets->AddSprite(asset.Name, textureName, uMin, vMin, uMax, vMax);
  //     if (id == 0)
  //     {
  //         GPC_ERROR << "MetaLoader: Sprite creation failed '" << asset.Name << "'" << ENDL;
  //         return MetaError::ASSET_LOAD_FAILED;
  //     }
  //
  //     return MetaError::SUCCESS;
  // }

  // MetaError MetaLoader::LoadSpriteSheet(const MetaAsset& asset) const
  // {
  //     size_t colonPos = asset.Path.find(':');
  //     if (colonPos == std::string::npos)
  //     {
  //         GPC_ERROR << "MetaLoader: Invalid sprite sheet format '" << asset.Path << "'" << ENDL;
  //         return MetaError::PARSE_ERROR;
  //     }
  //
  //     std::string textureName = asset.Path.substr(0, colonPos);
  //     std::string dims = asset.Path.substr(colonPos + 1);
  //
  //     uint32_t frameW, frameH;
  //     if (sscanf(dims.c_str(), "%u,%u", &frameW, &frameH) != 2)
  //     {
  //         GPC_ERROR << "MetaLoader: Invalid sprite sheet dims '" << dims << "'" << ENDL;
  //         return MetaError::PARSE_ERROR;
  //     }
  //
  //     AssetID id = m_pAssets->AddSpriteSheet(asset.Name, textureName, frameW, frameH);
  //     if (id == 0)
  //     {
  //         GPC_ERROR << "MetaLoader: Sprite sheet creation failed '" << asset.Name << "'" << ENDL;
  //         return MetaError::ASSET_LOAD_FAILED;
  //     }
  //
  //     return MetaError::SUCCESS;
  // }
  //
  // MetaError MetaLoader::LoadAnimation(const MetaAsset& asset) const
  // {
  //     size_t colonPos = asset.Path.find(':');
  //     if (colonPos == std::string::npos)
  //     {
  //         GPC_ERROR << "MetaLoader: Invalid animation format '" << asset.Path << "'" << ENDL;
  //         return MetaError::PARSE_ERROR;
  //     }
  //
  //     std::string sheetName = asset.Path.substr(0, colonPos);
  //     float frameDuration = std::stof(asset.Path.substr(colonPos + 1));
  //
  //     AssetID id = m_pAssets->AddAnimationFromSheet(asset.Name, sheetName, frameDuration);
  //     if (id == 0)
  //     {
  //         GPC_ERROR << "MetaLoader: Animation creation failed '" << asset.Name << "'" << ENDL;
  //         return MetaError::ASSET_LOAD_FAILED;
  //     }
  //
  //     return MetaError::SUCCESS;
  // }

  void MetaLoader::DeferAsset(const MetaAsset& asset)
  {
      if (m_pAssets->HasAsset(asset.Name))
      {
          GPC_INFO << "MetaLoader: Skipping existing deferred asset '" << asset.Name << "'" << ENDL;
          return;
      }

      MetaDeferredAsset deferred;
      deferred.Type = asset.Type;
      deferred.Name = asset.Name;
      deferred.Path = asset.Path;

      if (asset.Properties)
      {
          deferred.Properties = std::make_unique<SerialTree>();
          MetaReader::CopySerialTree(*asset.Properties, *deferred.Properties);
      }

      m_DeferredAssets.push_back(std::move(deferred));

      GPC_INFO << "MetaLoader: Deferred asset '" << asset.Name << "' for runtime resolution" << ENDL;
  }

  MetaError MetaLoader::ResolveRuntimeAssets(const MetaRuntimeContext& ctx)
  {
      if (!ctx.Window)
      {
          GPC_ERROR << "MetaLoader: Invalid runtime context (Window is null)" << ENDL;
          return MetaError::INVALID_CONTEXT;
      }

      for (const auto& deferred : m_DeferredAssets)
      {
          MetaError err = MetaError::SUCCESS;

          switch (deferred.Type)
          {
              case MetaAssetType::GRAPHIC_PROGRAM:
                  err = ResolveGraphicProgram(deferred, ctx);
                  break;
              case MetaAssetType::COMPUTE_PROGRAM:
                  err = ResolveComputeProgram(deferred, ctx);
                  break;
              case MetaAssetType::TEXTURE_ARRAY:
                  err = ResolveTextureArray(deferred, ctx);
                  break;
              default:
                  GPC_ERROR << "MetaLoader: Unknown deferred type for '" << deferred.Name << "'" << ENDL;
                  err = MetaError::UNKNOWN_ASSET_TYPE;
                  break;
          }

          if (err != MetaError::SUCCESS)
          {
              GPC_ERROR << "MetaLoader: Failed to resolve '" << deferred.Name << "'" << ENDL;
              return err;
          }
      }

      uint32_t count = static_cast<uint32_t>(m_DeferredAssets.size());
      m_DeferredAssets.clear();

      GPC_INFO << "MetaLoader: Resolved " << count << " runtime assets" << ENDL;
      return MetaError::SUCCESS;
  }

  MetaError MetaLoader::ResolveGraphicProgram(const MetaDeferredAsset& deferred, const MetaRuntimeContext& ctx)
  {
      size_t commaPos = deferred.Path.find(',');
      if (commaPos == std::string::npos)
      {
          GPC_ERROR << "MetaLoader: Invalid graphic program format '" << deferred.Path << "'" << ENDL;
          return MetaError::PARSE_ERROR;
      }

      std::string vertexName = deferred.Path.substr(0, commaPos);
      std::string fragmentName = deferred.Path.substr(commaPos + 1);

      ShaderAsset* vertex = m_pAssets->GetShader(vertexName);
      ShaderAsset* fragment = m_pAssets->GetShader(fragmentName);

      if (!vertex || !fragment)
      {
          GPC_ERROR << "MetaLoader: Shader dependency missing for '" << deferred.Name << "'" << ENDL;
          return MetaError::DEPENDENCY_MISSING;
      }

      CullMode cullMode = CullMode::FRONT;
      PolygonMode polygonMode = PolygonMode::MODE_FILL;

      if (deferred.Properties)
      {
          const SerialTree& props = *deferred.Properties;
          if (props.HasKey("cullMode"))
              cullMode = static_cast<CullMode>(*props["cullMode"].Get<uint8_t>());
          if (props.HasKey("polygonMode"))
              polygonMode = static_cast<PolygonMode>(*props["polygonMode"].Get<uint8_t>());
      }

      auto program = m_pAssets->AddGraphicProgram(
          deferred.Name,
          vertex,
          fragment,
          cullMode,
          polygonMode,
          {{
              .BindingDescription = Vertex::GetBindingDescription(),
              .AttributeDescriptions = Vertex::GetAttributeDescriptions()
          }},
          {
              DescriptorSetInformation::ForCamera(0),
              DescriptorSetInformation::ForTransform(1),
              DescriptorSetInformation::ForMaterials(2),
              DescriptorSetInformation::ForLights(3),
              GPC::DescriptorSetInformation {
                .Name = "images",
                .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                .BindingSlot = 4,
                .DescriptorCount = 50,
                .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .Sampler_ = nullptr
              },
              GPC::DescriptorSetInformation {
                        .Name = "LightTransform",
                        .Type = DescriptorType::UNIFORM_BUFFER,
                        .Usages = UNIFORM_BUFFER_BIT,
                        .BindingSlot = 5,
                        .DescriptorCount = 1,
                        .ElementCount = 1,
                        .ElementStride = sizeof(TransformLight),
                        .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .Sampler_ = nullptr
                    },
                    GPC::DescriptorSetInformation {
                         .Name = "shadowSpotLightMapDepth",
                         .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                         .BindingSlot = 6,
                         .DescriptorCount = 1,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr
                    },
                    GPC::DescriptorSetInformation {
                         .Name = "shadowPointLightMapDepth",
                         .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                         .BindingSlot = 7,
                         .DescriptorCount = 1,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr,
                    },
                    GPC::DescriptorSetInformation {
                         .Name = "shadowDirLightMapDepth",
                         .Type = GPC::DescriptorType::SAMPLED_IMAGE,
                         .BindingSlot = 8,
                         .DescriptorCount = 1,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr,
                    }
          },
          GRAPHICS_3D,
          ctx.Window
      );

      if (program == nullptr)
      {
          GPC_ERROR << "MetaLoader: GraphicProgram creation failed '" << deferred.Name << "'" << ENDL;
          return MetaError::ASSET_LOAD_FAILED;
      }

      return MetaError::SUCCESS;
  }

  MetaError MetaLoader::ResolveComputeProgram(const MetaDeferredAsset& deferred, const MetaRuntimeContext& ctx)
  {
      ShaderAsset* compute = m_pAssets->GetShader(deferred.Path);

      if (!compute)
      {
          GPC_ERROR << "MetaLoader: Compute shader missing '" << deferred.Path << "'" << ENDL;
          return MetaError::DEPENDENCY_MISSING;
      }

      ParticleComputeInformation pInfo {}; // TODO Add data in the deferred asset

      if (m_pAssets->AddParticleProgram(deferred.Name, pInfo, compute) == nullptr)
      {
          GPC_ERROR << "MetaLoader: ComputeProgram creation failed '" << deferred.Name << "'" << ENDL;
          return MetaError::ASSET_LOAD_FAILED;
      }

      return MetaError::SUCCESS;
  }

  MetaError MetaLoader::ResolveTextureArray(const MetaDeferredAsset& deferred, const MetaRuntimeContext& ctx)
  {
      size_t colonPos = deferred.Path.rfind(':');
      if (colonPos == std::string::npos)
      {
          GPC_ERROR << "MetaLoader: Invalid TextureArray format '" << deferred.Path << "'" << ENDL;
          return MetaError::PARSE_ERROR;
      }

      std::string texturesPart = deferred.Path.substr(0, colonPos);
      std::string dimsPart = deferred.Path.substr(colonPos + 1);

      uint32_t width, height;
      if (sscanf(dimsPart.c_str(), "%u,%u", &width, &height) != 2)
      {
          GPC_ERROR << "MetaLoader: Invalid TextureArray dims '" << dimsPart << "'" << ENDL;
          return MetaError::PARSE_ERROR;
      }

      std::vector<TextureAsset*> textures;
      std::stringstream ss(texturesPart);
      std::string texName;
      while (std::getline(ss, texName, ','))
      {
          Asset* asset = m_pAssets->GetAssets(texName);
          if (!asset)
          {
              GPC_ERROR << "MetaLoader: Texture '" << texName << "' not found for array" << ENDL;
              return MetaError::DEPENDENCY_MISSING;
          }
          textures.push_back(asset->ToTexture());
      }

      if (m_pAssets->AddTextureArray(deferred.Name, width, height, textures) == nullptr)
      {
          GPC_ERROR << "MetaLoader: TextureArray creation failed '" << deferred.Name << "'" << ENDL;
          return MetaError::ASSET_LOAD_FAILED;
      }

      return MetaError::SUCCESS;
  }

  MetaError MetaLoader::LoadInclude(const MetaAsset& asset, const std::string& currentMetaPath)
  {
      fs::path currentDir = fs::path(currentMetaPath).parent_path();
      fs::path includePath = currentDir / asset.Path;

      if (!fs::exists(includePath))
      {
          includePath = fs::path(asset.Path);
          if (!fs::exists(includePath))
          {
              GPC_ERROR << "MetaLoader: Include not found '" << asset.Path << "'" << ENDL;
              return MetaError::FILE_NOT_FOUND;
          }
      }

      GPC_INFO << "MetaLoader: Including '" << includePath.string() << "'" << ENDL;

      return Load(includePath.string());
  }

  } // GPC