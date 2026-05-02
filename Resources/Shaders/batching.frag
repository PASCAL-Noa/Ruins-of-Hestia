#version 450

struct Material {
    vec4    Tint;
    float   Metalic;
    float   Roughness;
    float   AO;
    float   EmissiveStrength;
    uint    TextureArray;
    uint    Diffuse;
    uint    NormalMap;
    uint    HeightMap;
};

layout(location = 0) in vec4 fragTint;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in Material material;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2DArray texSampler[50];

void main() {
    vec4 texColor   = texture(texSampler[material.TextureArray], vec3(fragTexCoord, float(material.Diffuse)));

    outColor = texColor * fragTint;
}