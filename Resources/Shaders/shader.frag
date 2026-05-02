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

const float PI = 3.14159265359;

layout(set = 0, binding = 4) uniform sampler2DArray texSampler[50];

layout(set = 0, binding = 5) uniform LightTransform
{
    mat4 lightSpaceMatrix[50];
};

layout(set = 0, binding = 6) uniform sampler2DArray         shadowSpotLightMapDepth;
layout(set = 0, binding = 7) uniform samplerCubeArrayShadow shadowPointLightMapDepth;
layout(set = 0, binding = 8) uniform sampler2DArray         shadowDirLightMapDepth;


float ShadowCalculation(vec3 normal, vec3 lightDir, uint lightIndex, uint layer)
{
    vec4 fragPosLightSpace = lightSpaceMatrix[lightIndex] * vec4(worldPos, 1.0);

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.z > 1.0)
    return 0.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
    return 0.0;

    float currentDepth = projCoords.z;
    float ndotl = max(dot(normalize(normal), normalize(lightDir)), 0.0);
    float bias = max(0.0005 * (1.0 - ndotl), 0.0001);

    vec2 texelSize = 1.0 / vec2(textureSize(shadowSpotLightMapDepth, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowSpotLightMapDepth, vec3(projCoords.xy + vec2(x, y) * texelSize, float(layer))).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}
// ----------------------------------------------------------------------------
float ShadowCalculationDirectional(vec3 normal, vec3 lightDir, uint lightIndex)
{
    vec4 fragPosLightSpace = lightSpaceMatrix[lightIndex] * vec4(worldPos, 1.0);

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    float ndotl = max(dot(normalize(normal), normalize(lightDir)), 0.0);
    float bias = max(0.0005 * (1.0 - ndotl), 0.0001);

    vec2 texelSize = 1.0 / vec2(textureSize(shadowDirLightMapDepth, 0));

    float shadow = 0.0;

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(
            shadowDirLightMapDepth,
            vec3(projCoords.xy + vec2(x, y) * texelSize, float(lightIndex))
            ).r;

            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    return shadow / 25.0f;
}
// ----------------------------------------------------------------------------
float ShadowCalculationPoint(uint globalLightIndex, uint cubeLayerIndex)
{
    Light light = globalLightBuffer.lights[globalLightIndex];
    vec3  lightToFrag   = worldPos - light.position.xyz;
    float currentDepth = length(lightToFrag);
    float farPlane     = light.range;
    vec3  dir          = normalize(lightToFrag);

    float ndotl = max(dot(normalize(worldNormal), -dir), 0.0);
    float bias = max(farPlane * mix(0.008, 0.002, ndotl), 0.05);

    float shadow     = 0.0;
    int   samples    = 20;
    float diskRadius = (currentDepth / farPlane) * 0.1;

    for (int s = 0; s < samples; ++s)
    {
        float noise = fract(sin(dot(worldPos.xy + float(s), vec2(12.9898, 78.233))) * 43758.5453);
        vec3 offset = vec3(
        sin(float(s) * 12.9898),
        cos(float(s) * 78.233),
        sin(float(s) * 37.719)
        ) * diskRadius;

        vec3 sampleDir = normalize(dir + offset);

        float result = texture(
        shadowPointLightMapDepth,
        vec4(sampleDir, float(cubeLayerIndex)),
        (currentDepth - bias) / farPlane
        );

        shadow += 1.0 - result;
    }

    return shadow / float(samples);
}
// ----------------------------------------------------------------------------
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texSampler[material.TextureArray], vec3(fragTexCoord, float(material.NormalMap))).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(worldPos);
    vec3 Q2  = dFdy(worldPos);
    vec2 st1 = dFdx(fragTexCoord);
    vec2 st2 = dFdy(fragTexCoord);

    vec3 N   = normalize(worldNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

int dirSpotIndex = 0;
int pointIndex   = 0;
int spotIndex    = 0;

void main()
{
    vec3 totalLight     = vec3(0.05);

    vec4    tex_color   = texture(texSampler[material.TextureArray], vec3(fragTexCoord, float(material.Diffuse)));
    vec3    diffuse     = pow(tex_color.rgb * (1 - material.Tint.a) + material.Tint.rgb * material.Tint.a, vec3(1));
    float   metallic    = material.Metalic;
    float   roughness   = material.Roughness;
    float   ao          = material.AO;


    vec3 N;
    if (material.NormalMap != material.Diffuse) {
        N = getNormalFromMap();
    } else {
        N = normalize(worldNormal);
    }

    vec3 ToCam  = normalize(camPos.xyz - worldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, diffuse, metallic);

    // reflectance equation
    vec3 LightColorIntensity = vec3(0.0);
    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < globalLightBuffer.lightCount; ++i){
        Light light = globalLightBuffer.lights[i];
        vec3 ToLight = normalize(light.position.xyz - worldPos);

        float shadow = 0.0f;

        float IsDirectionalLight    = float(globalLightBuffer.lights[i].type == 0);
        float IsPointLight          = float(globalLightBuffer.lights[i].type == 1);
        float IsSpotLight           = float(globalLightBuffer.lights[i].type == 2);

        /////////////// --------------
        // calculate per-light radiance
        vec3    H = normalize(ToCam + ToLight);
        float   distance = length(light.position.xyz - worldPos);
        float   attenuation = 1.0 / (distance * distance);
        vec3    radiance = light.color.xyz * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, ToCam, ToLight, roughness);
        vec3 F    = fresnelSchlick(clamp(dot(H, ToCam), 0.0, 1.0), F0);

        vec3    numerator    = NDF * G * F;
        float   denominator = 4.0 * max(dot(N, ToCam), 0.0) * max(dot(N, ToLight), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3    specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, ToLight), 0.0);

        // === Point Light ===
        float distanceToLight = length(light.position.xyz - worldPos);
        vec3 ToLightPointLight = ToLight * light.range / distanceToLight;
        shadow = ShadowCalculationPoint(i, uint(pointIndex));
        //Lo.xyz += IsPointLight * light.color.xyz * (kD * diffuse / PI + specular) * radiance * NdotL * (1 - shadow);  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        float pointFalloff = clamp(1.0 - distanceToLight / light.range, 0.0, 1.0);
        pointFalloff *= pointFalloff;
        LightColorIntensity.xyz += IsPointLight * light.color.xyz * max(0.0, dot(worldNormal, ToLight)) * light.intensity * pointFalloff * (1.0 - shadow);

        // === Directional Light ===
        vec3 ToLightDirectional = normalize(-light.direction.xyz);
        shadow = ShadowCalculationDirectional(worldNormal, ToLightDirectional, dirSpotIndex);
        LightColorIntensity.xyz += IsDirectionalLight * light.color.xyz * max(0.0, dot(worldNormal, ToLightDirectional)) * light.intensity * (1.0 - shadow);



        // === Spot Light ===
        vec3 L = normalize(light.position.xyz - worldPos);
        vec3 spotDir = normalize(-light.direction.xyz);
        float theta = dot(L, spotDir);
        float epsilon = light.innerCone - light.outerCone;
        float intensity = clamp((theta - light.outerCone) / epsilon, 0.0, 1.0);
        float falloff = clamp(1.0 - distanceToLight / light.range, 0.0, 1.0);
        falloff = falloff * falloff * falloff; // Quadratic falloff
        shadow = ShadowCalculation(worldNormal, normalize(-light.direction.xyz), dirSpotIndex, spotIndex);
        LightColorIntensity.xyz += IsSpotLight * light.color.xyz * max(0.0, dot(worldNormal, ToLight)) * intensity * falloff * light.intensity * (1.0 - shadow);

        pointIndex      += int(IsPointLight);
        dirSpotIndex    += int(IsDirectionalLight) + int(IsSpotLight);
        spotIndex       += int(IsSpotLight);

    }

    vec3 finalColor = Lo.xyz;

    // HDR tonemapping
    finalColor = finalColor / (finalColor + vec3(1.0));
    // gamma correct
    finalColor = pow(finalColor, vec3(1.0/2.2));

    // outColor = texColor * vec4(worldNormal, 1.0);
    if (tex_color.a < 0.8) {
        discard;
    }
    outColor = vec4(finalColor + (diffuse * totalLight * LightColorIntensity), tex_color.a);

    vec3 mix = outColor.rgb * (1 - material.Tint.a) + material.Tint.rgb * material.Tint.a;
    outColor = vec4(mix, outColor.a);

    //LIGHT DEBUG
    //outColor = vec4((totalLight * LightColorIntensity), 1.0);

    //NORMAL DEBUG
    // outColor = texColor * vec4(worldNormal, 1.0);
}