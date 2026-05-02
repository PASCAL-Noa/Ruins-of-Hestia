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

layout(set = 0, binding = 0) uniform ViewBuffer {
    mat4 viewproj;
    mat4 view;
    mat4 proj;
} viewBuffer;

layout(set = 0, binding = 1) readonly buffer TransformStorage {
    mat4 transforms[];
} tStorage;

layout(set = 0, binding = 2) readonly buffer MaterialStorage {
    Material materials[];
} mStorage;

layout(push_constant) uniform ObjectIndex {
    uint objectID;
} index;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

void main() {

    mat4 transform = tStorage.transforms[index.objectID];
    Material material = mStorage.materials[index.objectID];

    vec4 positionWorld = transform * vec4(position, 1.0);
    gl_Position = viewBuffer.viewproj * positionWorld;

    outColor = vec4(material.Tint.xyz * material.EmissiveStrength, 1.0);

}