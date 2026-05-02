#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(set = 0, binding = 1) uniform PointLightPositions {
    mat4 lightSpaceMatrices[16 * 6];
};

layout(push_constant) uniform PushConstant {
    uint  objectIndex;
    uint  faceOffset;
    float farPlane;
    float padding;
    vec3  lightPos;
};

layout(location = 0) in  vec3 inWorldPos[];
layout(location = 0) out vec3 outWorldPos;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = int(faceOffset) + face;

        mat4 lightMatrix = lightSpaceMatrices[faceOffset + face];

        for (int v = 0; v < 3; ++v)
        {
            outWorldPos = inWorldPos[v];
            gl_Position = lightMatrix * vec4(inWorldPos[v], 1.0);
            EmitVertex();
        }

        EndPrimitive();
    }
}