SamplerState mainTexSampler : register(s0);
SamplerComparisonState shadowSampler : register(s1);

Texture2D mainTex : register(t0);
Texture2D normalMap : register(t1);
Texture2D shadowMap : register(t3);

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
    PointLight pointLights[2];
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
    
    float4 lightSpacePos : TEXCOORD1;
};

float CalculateShadowFactor(float4 lightSpacePos)
{
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = projCoords.y * -0.5f + 0.5f;
    
    if (projCoords.x < 0.0f || projCoords.x > 1.0f || projCoords.y < 0.0f || projCoords.y > 1.0f) return 1.0f;
    
    float shadowFactor = shadowMap.SampleCmpLevelZero(shadowSampler, projCoords.xy, projCoords.z);
    
    return shadowFactor;
}

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = mainTex.Sample(mainTexSampler, input.uv);
    
    float distanceFromCamera = length(cameraPos.xyz - input.posWorld.xyz);
    float fogFactor = saturate(distanceFromCamera / ambientFog.w);
    float4 fogColor = float4(ambientFog.xyz, 1.0f);
    
    float3 normalMapSample = normalMap.Sample(mainTexSampler, input.uv).xyz * 2.0f - 1.0f;
    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.norm);
    float3 worldNormal = normalize(mul(normalMapSample, TBN));
    
    float shadowFactor = CalculateShadowFactor(input.lightSpacePos);
    float4 finalLight = input.light;
    finalLight.rgb = lerp(finalLight.rgb * 0.3f, finalLight.rgb, shadowFactor);
    
    float4 pointLightContribution = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [unroll]
    for (int i = 0; i < 2; i++)
    {
        float3 vecToLight = pointLights[i].worldPos.xyz - input.posWorld.xyz;
        
        float distance = length(vecToLight);
        if (distance > pointLights[i].range) continue;
        
        vecToLight = vecToLight / distance;
        
        float spot = pow(max(dot(-vecToLight, pointLights[i].directionAndAngle.xyz), 1e-5f), pointLights[i].directionAndAngle.w);
        if (spot < 1e-5f) continue;
        
        float3 attenuateConstants = float3(pointLights[i].aConstant, pointLights[i].aLinear, pointLights[i].aQuadratic);
        float attenuation = spot / dot(attenuateConstants, float3(1.0f, distance, distance * distance));
        
        float diffuseFactor = saturate(dot(worldNormal, vecToLight));
        if (diffuseFactor < 1e-5f) continue;
        
        float4 pointLightColor = pointLights[i].color * diffuseFactor;
        
        float3 viewDirection = normalize(cameraPos.xyz - input.posWorld.xyz);
        float3 blinnPhongHalfVector = normalize(vecToLight + viewDirection);
        float specularFactor = dot(worldNormal, blinnPhongHalfVector);
        
        pointLightColor += pointLights[i].color * specularFactor;
        
        pointLightContribution += pointLightColor * attenuation;
    }
    
    float4 finalColor = texColor * (finalLight + pointLightContribution);
    
    return lerp(finalColor, fogColor, fogFactor);
}