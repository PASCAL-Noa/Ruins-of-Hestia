#version 450

layout(binding = 1) uniform sampler2DArray renderTexture[50];

layout(location = 0) in vec4 uiTint;
layout(location = 1) in vec3 texCoords;
layout(location = 2) in flat uint textureIndex;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 color = texture(renderTexture[textureIndex], texCoords);
    outColor = uiTint * color;
}