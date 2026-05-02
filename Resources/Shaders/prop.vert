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

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
    mat4 viewproj;
    mat4 view;
    mat4 proj;
} viewBuffer;

layout(set = 0, binding = 1) readonly buffer MaterialStorage {
    Material materials[];
} mStorage;

layout(push_constant) uniform ObjectIndex {
    uint objectID;
} index;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

layout(location = 3) in vec4 WorldPosition;
layout(location = 4) in vec4 WorldRotation;
layout(location = 5) in vec4 Size;
layout(location = 6) in vec4 Tint;

layout(location = 0) out vec4 fragTint;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint Offset;
layout(location = 3) out Material fragMaterial;

void main() {

    uint id = index.objectID;
    Material material = mStorage.materials[id];

    vec4 q = WorldRotation;
    mat4 rMatrix = mat4(
        1.0 - 2.0 * (q.y*q.y + q.z*q.z),    2.0 * (q.x*q.y + q.z*q.w),          2.0 * (q.x*q.z - q.y*q.w),          0.0,
        2.0 * (q.x*q.y - q.z*q.w),          1.0 - 2.0 * (q.x*q.x + q.z*q.z),    2.0 * (q.y*q.z + q.x*q.w),          0.0,
        2.0 * (q.x*q.z + q.y*q.w),          2.0 * (q.y*q.z - q.x*q.w),          1.0 - 2.0 * (q.x*q.x + q.y*q.y),    0.0,
        0.0,                                0.0,                                0.0,                                1.0
    );

    vec4 positionWorld = rMatrix * vec4(Position * Size.xyz, 1.0) + WorldPosition.xyzw;
    positionWorld.w = 1.0;
    gl_Position = viewBuffer.viewproj * positionWorld;

    fragTint        = Tint;
    fragTexCoord    = TexCoord;
    fragMaterial    = material;
    Offset          = uint(WorldPosition.w);
}