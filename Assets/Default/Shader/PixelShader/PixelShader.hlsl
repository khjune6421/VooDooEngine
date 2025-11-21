SamplerComparisonState shadowSampler : register(s0);
SamplerState defaultTexSampler : register(s1);

Texture2D shadowMap : register(t0);
TextureCubeArray cubeShadowMapArray : register(t1);

Texture2D textureArray[2] : register(t2); // t2 = diffuse map, t3 = normal map, etc.

struct PointLight
{
    float4 worldPos;
    float4 color;
    
    float4 directionAndAngle; // Only for spotlights: xyz = direction, w = angle exponent
    
    float range;
    
    // Attenuation factors
    float aConstant;
    float aLinear;
    float aQuadratic;
};


cbuffer AmbientLightConstBuffer : register(b0)
{
    float4 ambientLight;
}
cbuffer DirectionalLightShadowConstBuffer : register(b1)
{
    matrix directionalLightVP;
}
cbuffer PointLightConstBuffer : register(b2)
{
    PointLight pointLights[8];
    
    uint pointLightCount;
    uint padding[3];
}
cbuffer CameraConstBuffer : register(b3)
{
    float4 cameraPos;
}
cbuffer AmbientFogConstBuffer : register(b4)
{
    float4 ambientFog; // w value is range
}

float CalculateDirectionalShadow(float4 worldPos)
{
    float4 lightSpacePos = mul(directionalLightVP, worldPos);
    if (lightSpacePos.w <= 0.0f) return 1.0f;
    lightSpacePos.xyz /= lightSpacePos.w;
    
    float2 shadowTexCoord;
    shadowTexCoord.x = lightSpacePos.x * 0.5f + 0.5f;
    shadowTexCoord.y = -lightSpacePos.y * 0.5f + 0.5f;
    
    if (shadowTexCoord.x < 0.0f || shadowTexCoord.x > 1.0f || shadowTexCoord.y < 0.0f || shadowTexCoord.y > 1.0f) return 1.0f; // Not in shadow
    
    // Bias to prevent shadow acne
    float bias = 0.999f; // Need to find a sweetspot
    float currentDepth = lightSpacePos.z * bias;
    
    float shadow = shadowMap.SampleCmpLevelZero(shadowSampler, shadowTexCoord, currentDepth);
    
    return shadow;
}

float4 CalculatePointLight(uint index, float3 worldPos, float3 worldNormal, float3 viewDirection)
{
    PointLight pLight = pointLights[index];
    
    float3 vecToLight = pLight.worldPos.xyz - worldPos;
    float distanceSq = dot(vecToLight, vecToLight);
    float distance = sqrt(distanceSq);
    if (distance > pLight.range) return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Shadow
    float shadowFactor = cubeShadowMapArray.SampleCmpLevelZero(shadowSampler, float4(-vecToLight, index), distance / pLight.range);
    if (shadowFactor <= 0.0f) return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    vecToLight /= distance; // Normalize
    
    float spotDot = abs(dot(-vecToLight, pLight.directionAndAngle.xyz));
    if (spotDot < 0.0f) return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float spot = pow(spotDot, pLight.directionAndAngle.w);
    
    float diffuseFactor = dot(worldNormal, vecToLight);
    if (diffuseFactor < 0.0f) return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    diffuseFactor = saturate(diffuseFactor);
    
    float attenuation = spot / (pLight.aConstant + pLight.aLinear * distance + pLight.aQuadratic * distanceSq);
    float4 result = pLight.color * diffuseFactor;
    
    float3 halfVector = normalize(vecToLight + viewDirection);
    float specularFactor = pow(saturate(dot(worldNormal, halfVector)), 32.0f); // pow value is shininess
    
    result += pLight.color * specularFactor;
    
    return result * attenuation;
}

struct PSInput
{
    float4 pos : SV_POSITION0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
    
    float4 posWorld : WORLDPOS0;
    float4 light : COLOR0;
    float3 bitangent : BITANGENT0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = textureArray[0].Sample(defaultTexSampler, input.uv);
    float3 normalMapSample = textureArray[1].Sample(defaultTexSampler, input.uv).xyz;
    
    normalMapSample = normalMapSample * 2.0f - 1.0f;
    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.norm);
    float3 worldNormal = normalize(mul(normalMapSample, TBN));
    
    float3 vecToCamera = cameraPos.xyz - input.posWorld.xyz;
    float distanceFromCameraSq = dot(vecToCamera, vecToCamera);
    float distanceFromCamera = sqrt(distanceFromCameraSq);
    float3 viewDirection = vecToCamera / distanceFromCamera;
    
    input.light *= CalculateDirectionalShadow(input.posWorld);
    
    [loop]
    for (uint i = 0; i < pointLightCount; i++) input.light += CalculatePointLight(i, input.posWorld.xyz, worldNormal, viewDirection);
    
    input.light += ambientLight;
    
    float fogFactor = pow(saturate(distanceFromCamera / ambientFog.w), 2.0f);
    float4 fogColor = float4(ambientFog.xyz, 1.0f);
    
    return lerp(texColor * input.light, fogColor, fogFactor);
}