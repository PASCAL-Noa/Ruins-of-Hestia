#version 450

layout(triangles, invocations = 8) in;
layout(triangle_strip, max_vertices = 3) out;

layout(set = 0, binding = 1) uniform LightTransform {
    mat4 lightVP[8];
} light;

void main()
{
    gl_Layer = gl_InvocationID;

    for (int i = 0; i < 3; i++)
    {
        gl_Position = light.lightVP[gl_InvocationID] * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}