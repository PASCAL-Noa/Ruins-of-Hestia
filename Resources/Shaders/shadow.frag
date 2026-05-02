#version 450

layout(location = 0) in vec2 textureCoord;
layout(location = 1) in flat uint array;
layout(location = 2) in flat uint tex;

layout(set = 0, binding = 2) uniform sampler2DArray texSampler[50];

void main()
{
    vec4 tex_color = texture(texSampler[array], vec3(textureCoord, tex));

    if (tex_color.a < 0.8) discard;
}