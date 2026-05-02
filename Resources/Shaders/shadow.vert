#version 450

layout(std430, set = 0, binding = 0) readonly buffer Transforms {
    mat4 model[];
} transforms;

layout(set = 0, binding = 1) uniform LightTransform {
    mat4 lightVP[];
} light;

layout(push_constant) uniform ObjectData {
    uint objectID;
    uint lightBaseIndex;
    uint textureArray;
    uint diffuse;
} obj;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

layout(location = 0) out vec2 outFrag;
layout(location = 1) out uint outArray;
layout(location = 2) out uint outTexture;

void main()
{
    mat4 transform = transforms.model[obj.objectID];
    gl_Position = transform * vec4(position, 1.0);

    outFrag = texCoords;
    outArray = obj.textureArray;
    outTexture = obj.diffuse;
}