#version 450

layout(set = 0, binding = 0) uniform ViewBuffer {
    mat4 viewproj;
    mat4 view;
    mat4 proj;
} viewBuffer;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

layout(location = 3) in vec4 color;
layout(location = 4) in vec2 screenPosition;
layout(location = 5) in vec2 size;
layout(location = 6) in uint textureIndex;
layout(location = 7) in uint textureLayerOffset;
layout(location = 8) in float rotation;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 texCoordsOut;
layout(location = 2) out flat uint textureIndexOut;

void main()
{

    vec2 screenSize = vec2(viewBuffer.view[0][0] * 0.5, viewBuffer.view[0][1] * 0.5f);

    float sposX          = (screenPosition.x - screenSize.x) / screenSize.x;
    float sposY          = (screenPosition.y - screenSize.y) / screenSize.y;

    float scaleX         = size.x  / screenSize.x;
    float scaleY         = size.y  / screenSize.y;

    sposY -= scaleY * 0.5;

    float c = cos(rotation);
    float s = sin(rotation);
    vec2 rotated = vec2(position.x * c - position.y * s, position.x * s + position.y * c);

    gl_Position     = vec4(rotated.x * scaleX + sposX, rotated.y * scaleY + sposY, 1.0, 1.0);
    texCoordsOut    = vec3(texCoords, textureLayerOffset);
    outColor        = color;
    textureIndexOut = textureIndex;
}
