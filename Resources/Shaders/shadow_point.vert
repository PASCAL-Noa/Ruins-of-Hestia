#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(set = 0, binding = 0) readonly buffer TransformBuffer {
    mat4 transforms[];
};

layout(push_constant) uniform PushConstant {
    uint  objectIndex;
    uint  faceOffset;
    float farPlane;
    float padding;
    vec3  lightPos;
};

layout(location = 0) out vec3 outWorldPos;

void main()
{
    vec4 worldPos = transforms[objectIndex] * vec4(inPosition, 1.0);
    outWorldPos   = worldPos.xyz;
    gl_Position   = worldPos; // Le geometry shader appliquera la projection
}