#version 450

layout(location = 0) in vec3 inWorldPos;

layout(push_constant) uniform PushConstant {
    uint  objectIndex;
    uint  faceOffset;
    float farPlane;
    float padding;
    vec3  lightPos;
};

void main()
{
    float dist = length(inWorldPos - lightPos);
    gl_FragDepth = dist / farPlane;
}