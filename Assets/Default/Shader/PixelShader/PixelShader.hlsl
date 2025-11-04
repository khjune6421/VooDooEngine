SamplerState defaultTexSampler : register(s0);
SamplerComparisonState shadowSampler : register(s1);

Texture2D shadowMap : register(t0);
Texture2D mainTex : register(t1);
Texture2D normalMap : register(t2);

cbuffer CameraConstBuffer : register(b0)
{
    float4 cameraPos;
}

cbuffer AmbientFogConstBuffer : register(b1)
{
    float4 ambientFog; // w value is range
}

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
cbuffer PointLightConstBuffer : register(b2)
{
    PointLight pointLights[8];
    
    uint numPointLights;
    uint padding[3];
}

cbuffer ShadowConstBuffer : register(b3)
{
    matrix lightVP;
}

struct PSInput
{
    float4 pos : SV_POSITION0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
    
    float4 posWorld : WORLDPOS0;
    float4 light : COLOR1;
    float3 bitangent : BITANGENT0;
};

float4 CalculatePointLight(PointLight light, float3 worldPos, float3 worldNormal, float3 viewDirection)
{
    float3 vecToLight = light.worldPos.xyz - worldPos;
    float distanceSq = dot(vecToLight, vecToLight);
    float distance = sqrt(distanceSq);
    
    if (distance > light.range) return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float rcpDistance = rcp(distance);
    vecToLight *= rcpDistance;
    
    float spotDot = dot(-vecToLight, light.directionAndAngle.xyz);
    if (spotDot < 1e-5f) return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float spot = pow(spotDot, light.directionAndAngle.w);
    
    float diffuseFactor = dot(worldNormal, vecToLight);
    if (diffuseFactor < 1e-5f) return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    diffuseFactor = saturate(diffuseFactor);
    
    float attenuation = spot * rcp(light.aConstant + light.aLinear * distance + light.aQuadratic * distanceSq);
    float4 result = light.color * diffuseFactor;
    
    float3 halfVector = normalize(vecToLight + viewDirection);
    float specularFactor = pow(saturate(dot(worldNormal, halfVector)), 32.0f); // pow value is shininess
    
    result += light.color * specularFactor;
    
    return result * attenuation;
}

float CalculateShadow(float4 worldPos)
{
    float4 lightSpacePos = mul(worldPos, lightVP);
    lightSpacePos.xyz /= lightSpacePos.w;
    
    float2 shadowTexCoord;
    shadowTexCoord.x = lightSpacePos.x * 0.5f + 0.5f;
    shadowTexCoord.y = -lightSpacePos.y * 0.5f + 0.5f;
    
    if (shadowTexCoord.x < 0.0f || shadowTexCoord.x > 1.0f || shadowTexCoord.y < 0.0f || shadowTexCoord.y > 1.0f) return 1.0f; // Not in shadow
    
    // Bias to prevent shadow acne
    float bias = 0.001f;
    float currentDepth = lightSpacePos.z - bias;
    
    float shadow = shadowMap.SampleCmpLevelZero(shadowSampler, shadowTexCoord, currentDepth);
    
    return shadow;
}

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = mainTex.Sample(defaultTexSampler, input.uv);
    float3 normalMapSample = normalMap.Sample(defaultTexSampler, input.uv).xyz;
    float shadowFactor = CalculateShadow(input.posWorld);
    
    normalMapSample = normalMapSample * 2.0f - 1.0f;
    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.norm);
    float3 worldNormal = normalize(mul(normalMapSample, TBN));
    
    float3 vecToCamera = cameraPos.xyz - input.posWorld.xyz;
    float distanceFromCameraSq = dot(vecToCamera, vecToCamera);
    float distanceFromCamera = sqrt(distanceFromCameraSq);
    float3 viewDirection = vecToCamera * rcp(distanceFromCamera);
    
    [loop]
    for (uint i = 0; i < numPointLights; i++) input.light += CalculatePointLight(pointLights[i], input.posWorld.xyz, worldNormal, viewDirection);
    
    float fogFactor = saturate(distanceFromCamera * rcp(ambientFog.w));
    float4 fogColor = float4(ambientFog.xyz, 1.0f);
    
    float4 finalColor = texColor * input.light;
    finalColor.rgb *= shadowFactor;
    
    return lerp(finalColor, fogColor, fogFactor);
}