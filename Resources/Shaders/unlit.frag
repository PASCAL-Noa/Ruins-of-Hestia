#version 450

#define MAX_LIGHTS 100

struct Material {
    vec4    Tint;
    float   Metalic;
    float   Roughness;
    float   AO;
    float   EmissiveStrength;
    uint    TextureArray;
    uint    Diffuse;
    uint    NormalMap;
    uint    HeightMap;
};


struct Light
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 ambient;

    float intensity;
    float range;
    float innerCone;
    float outerCone;

    uint type;
    uint padding1;
    uint padding2;
    uint padding3;
};

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
    mat4 viewproj;
    mat4 view;
    mat4 proj;
} viewBuffer;

layout(set = 0, binding = 3) uniform UniformLightBuffer
{
    uint lightCount;
    Light lights[MAX_LIGHTS];
} globalLightBuffer;


layout(location = 0) in vec4 camPos;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 worldNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 4) flat in Material material;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 4) uniform sampler2DArray texSampler[50];

layout(set = 0, binding = 5) uniform LightTransform
{
    mat4 lightSpaceMatrix[50];
};

layout(set = 0, binding = 6) uniform sampler2DArray         shadowSpotLightMapDepth;
layout(set = 0, binding = 7) uniform samplerCubeArrayShadow shadowPointLightMapDepth;
layout(set = 0, binding = 8) uniform sampler2DArray         shadowDirLightMapDepth;

void main()
{
    vec3 totalLight     = vec3(0.05);

    vec4    tex_color   = texture(texSampler[material.TextureArray], vec3(fragTexCoord, float(material.Diffuse)));
    vec3    diffuse     = pow(tex_color.rgb * (1 - material.Tint.a) + material.Tint.rgb * material.Tint.a, vec3(1));


    vec3 finalColor = diffuse.rgb;

    // HDR tonemapping
    finalColor = finalColor / (finalColor + vec3(1.0));
    // gamma correct
    finalColor = pow(finalColor, vec3(1.0/2.2));

    // outColor = texColor * vec4(worldNormal, 1.0);
    if (tex_color.a < 0.8) {
        discard;
    }
    outColor = vec4(finalColor, tex_color.a);

    vec3 mix = outColor.rgb * (1 - material.Tint.a) + material.Tint.rgb * material.Tint.a;
    outColor = vec4(mix, outColor.a);
}