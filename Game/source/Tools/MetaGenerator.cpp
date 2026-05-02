//
// Created by killi on 25/03/2026.
//

#include "Tools/MetaGenerator.h"
#include "Meta/MetaBuilder.h"

namespace Tools
{

void GenerateCommonPack()
{
    GPC::MetaBuilder::Create()
        .Shader("Default Vertex", "shader.vert")
        .Shader("Default Fragment", "shader.frag")
        .WriteTo("Resources/Packs/common.meta");
}

void GenerateSceneMetaPack()
{
    GPC::MetaBuilder::Create()
        .Shader("Meta Vertex", "shader.vert")
        .Shader("Meta Fragment", "shader.frag")
        .Texture("MetaTex", "Resources/Materials/debug.jpg")
        .Mesh("MetaCube", "cube")
        .Mesh("MetaSphere", "sphere")
        .Mesh("MetaPlane", "plane")
        .Light("MetaLight")
            .Type(GPC::Light::LightType::POINT)
            .Color(255, 255, 255)
            .Build()
        .GraphicProgram("Default Program")
            .Vertex("Meta Vertex")
            .Fragment("Meta Fragment")
            .CullMode(GPC::CullMode::FRONT)
            .Build()
        .TextureArray("MetaTextures")
            .Textures({"MetaTex"})
            .Size(1024, 1024)
            .Build()
        .WriteTo("Resources/Packs/scene_meta.meta");
}

void GenerateSceneDebugPack()
{
    GPC::MetaBuilder::Create()
        .Include("common.meta")
        .Texture("carbon", "Resources/Materials/carbon1024.jpg")
        .Texture("parquet", "Resources/Materials/parquet1024.jpg")
        .Light("RedLight")
            .Type(GPC::Light::LightType::POINT)
            .Color(255, 0, 0)
            .Build()
        .Light("GreenLight")
            .Type(GPC::Light::LightType::POINT)
            .Color(0, 255, 0)
            .Build()
        .Light("BlueLight")
            .Type(GPC::Light::LightType::POINT)
            .Color(0, 0, 255)
            .Build()
        .Light("WhiteLight")
            .Type(GPC::Light::LightType::POINT)
            .Color(255, 255, 255)
            .Build()
        .GraphicProgram("Default Program")
            .Vertex("Default Vertex")
            .Fragment("Default Fragment")
            .CullMode(GPC::CullMode::FRONT)
            .Build()
        .TextureArray("arr")
            .Textures({"carbon", "parquet"})
            .Size(1024, 1024)
            .Build()
        .WriteTo("Resources/Packs/scene_debug.meta");
}

void GenerateSceneDefaultPack()
{
    GPC::MetaBuilder::Create()
        .Include("common.meta")
        .Texture("Debug", "Resources/Materials/debug.jpg")
        .Texture("Debug2", "Resources/Materials/Debug2x2.png")
        .Texture("Debug2 Rotated", "Resources/Materials/Debug2x2_Rotated.png")
        .Texture("Debug3", "Resources/Materials/Debug3x3.png")
        .Texture("Parquet", "Resources/Materials/carbon1024.jpg")
        .Texture("Carbon", "Resources/Materials/parquet1024.jpg")
        .Texture("SelectorDebug", "Resources/Materials/SelectorDebug.png")
        .Texture("Blue", "Resources/Materials/blue.jpg")
        .Texture("Red", "Resources/Materials/red.jpg")
        .Mesh("Cube", "cube")
        .Mesh("Sphere", "sphere")
        .Mesh("Plane", "plane")
        .Light("PointRed")
            .Type(GPC::Light::LightType::POINT)
            .Color(255, 0, 0)
            .Intensity(1.0f)
            .Range(20.0f)
            .Build()
        .Light("PointGreen")
            .Type(GPC::Light::LightType::POINT)
            .Color(0, 255, 0)
            .Intensity(1.0f)
            .Range(20.0f)
            .Build()
        .Light("PointBlue")
            .Type(GPC::Light::LightType::POINT)
            .Color(0, 0, 255)
            .Intensity(1.0f)
            .Range(20.0f)
            .Build()
        .Light("PointWhite")
            .Type(GPC::Light::LightType::POINT)
            .Color(255, 255, 255)
            .Intensity(1.0f)
            .Range(20.0f)
            .Build()
        .Light("SpotBlue")
            .Type(GPC::Light::LightType::SPOT)
            .Color(0, 0, 255)
            .Intensity(300.0f)
            .Range(20.0f)
            .Build()
        .Light("DirWhite")
            .Type(GPC::Light::LightType::DIRECTIONAL)
            .Color(255, 255, 255)
            .Intensity(40.0f)
            .Build()
        .GraphicProgram("Default Program")
            .Vertex("Default Vertex")
            .Fragment("Default Fragment")
            .CullMode(GPC::CullMode::FRONT)
            .Build()
        .TextureArray("1024x1024")
            .Textures({"Debug", "Debug2", "Debug2 Rotated", "Debug3",
                       "Carbon", "Parquet", "SelectorDebug", "Blue", "Red"})
            .Size(1024, 1024)
            .Build()
        .WriteTo("Resources/Packs/scene_default.meta");
}

void GenerateSceneComputePack()
{
    GPC::MetaBuilder::Create()
        .Shader("Default Vertex", "shader.vert")
        .Shader("Default Fragment", "shader.frag")
        .Shader("Particle Comp", "particles.comp")
        .Texture("carbon", "Resources/Materials/carbon1024.jpg")
        .Texture("parquet", "Resources/Materials/parquet1024.jpg")
        .Mesh("Triangle", "triangle")
        .Mesh("Cube", "cube")
        .Mesh("Sphere", "sphere")
        .GraphicProgram("Default Program")
            .Vertex("Default Vertex")
            .Fragment("Default Fragment")
            .CullMode(GPC::CullMode::FRONT)
            .Build()
        .TextureArray("arr")
            .Textures({"parquet", "carbon"})
            .Size(1024, 1024)
            .Build()
        .WriteTo("Resources/Packs/scene_compute.meta");
}

void GenerateSceneCollisionPack()
{
    GPC::MetaBuilder::Create()
        .Include("common.meta")
        .Texture("Blue Debug", "Resources/Materials/blue.jpg")
        .Texture("Red Debug", "Resources/Materials/red.jpg")
        .Mesh("Cube", "cube")
        .Mesh("Sphere", "sphere")
        .Mesh("Plane", "plane")
        .Light("PointWhite")
            .Type(GPC::Light::LightType::POINT)
            .Color(255, 255, 255)
            .Intensity(1.0f)
            .Range(20.0f)
            .Build()
        .GraphicProgram("Default Program")
            .Vertex("Default Vertex")
            .Fragment("Default Fragment")
            .CullMode(GPC::CullMode::FRONT)
            .PolygonMode(GPC::PolygonMode::MODE_LINE)
            .Build()
        .TextureArray("Collision Texture")
            .Textures({"Blue Debug", "Red Debug"})
            .Size(1024, 1024)
            .Build()
        .WriteTo("Resources/Packs/scene_collision.meta");
}

void GenerateAllMetaPacks()
{
    GenerateCommonPack();
    GenerateSceneMetaPack();
    GenerateSceneDebugPack();
    GenerateSceneDefaultPack();
    GenerateSceneComputePack();
    GenerateSceneCollisionPack();
}

}
