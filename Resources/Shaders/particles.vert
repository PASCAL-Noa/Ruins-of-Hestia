#version 450

layout(set = 0, binding = 0) uniform ViewBuffer {
    mat4 viewproj;
    mat4 view;
    mat4 proj;
} viewBuffer;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

layout(location = 3) in vec4 inPosition;
layout(location = 4) in vec4 inColor;
layout(location = 5) in vec4 size;
layout(location = 6) in vec4 qRotation;

layout(push_constant) uniform ObjectIndex {
    mat4 transform;
} object;

layout(location = 0) out vec4 fragColor;

void main() {

    vec4 position = vec4(position.xyz * size.xyz, 1.0);
    vec4 q = normalize(qRotation);

    mat4 rMatrix = mat4(
        1.0 - 2.0 * (q.y*q.y + q.z*q.z),    2.0 * (q.x*q.y + q.z*q.w),          2.0 * (q.x*q.z - q.y*q.w),          0.0,
        2.0 * (q.x*q.y - q.z*q.w),          1.0 - 2.0 * (q.x*q.x + q.z*q.z),    2.0 * (q.y*q.z + q.x*q.w),          0.0,
        2.0 * (q.x*q.z + q.y*q.w),          2.0 * (q.y*q.z - q.x*q.w),          1.0 - 2.0 * (q.x*q.x + q.y*q.y),    0.0,
        0.0,                                0.0,                                0.0,                                1.0
    );

    position = rMatrix * position;
    position.xyz = position.xyz + inPosition.xyz;

    gl_Position = viewBuffer.viewproj * object.transform * position;
    fragColor = inColor.rgba;

}