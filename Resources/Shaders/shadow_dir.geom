#version 450

layout(triangles, invocations = 1) in;

layout(location = 0) in vec2 textureCoord[];
layout(location = 1) in flat uint array[];
layout(location = 2) in flat uint tex[];

layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) out vec2 outTextureCoord;
layout(location = 1) out flat uint outArray;
layout(location = 2) out flat uint outTex;

layout(set = 0, binding = 1) uniform LightTransform {
    mat4 lightVP[8];
} light;

void main()
{
    gl_Layer = 0;

    for (int i = 0; i < 3; i++)
    {
        gl_Position = light.lightVP[0] * gl_in[i].gl_Position;

        // Copier les attributs
        outTextureCoord = textureCoord[i];
        outArray = array[i];
        outTex = tex[i];

        EmitVertex();
    }
    EndPrimitive();
}