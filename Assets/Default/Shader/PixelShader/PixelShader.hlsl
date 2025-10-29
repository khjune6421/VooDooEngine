SamplerState mainTexSampler : register(s0);

Texture2D mainTex : register(t0);
Texture2D normalMap : register(t1);

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

cbuffer AmbientFogConstBuffer : register(b0)
{
    float4 cameraPos;
    float4 ambientFog; // w value is range
}

cbuffer PointLightConstBuffer : register(b1)
{
    PointLight pointLights[2];
}

cbuffer PointLightConstBuffer : register(b2)
{
    int season;
}

struct PSInput
{
    float4 pos : SV_POSITION0;
    float4 posWorld : WORLDPOS0;
    
    float4 col : COLOR0;
    float4 light : COLOR1;
    
    float3 norm : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BITANGENT0;
    
    float2 uv : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = mainTex.Sample(mainTexSampler, input.uv);
    clip(texColor.a - 1e-5f);
    
    float distanceFromCamera = length(cameraPos.xyz - input.posWorld.xyz);
    float fogFactor = saturate(distanceFromCamera / ambientFog.w);
    float4 fogColor = float4(ambientFog.xyz, 1.0f);
    
    float3 normalMapSample = normalMap.Sample(mainTexSampler, input.uv).xyz * 2.0f - 1.0f;
    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.norm);
    float3 worldNormal = normalize(mul(normalMapSample, TBN));
    
    [unroll]
    for (int i = 0; i < 2; i++)
    {
        float3 vecToLight = pointLights[i].worldPos.xyz - input.posWorld.xyz;
        
        float distance = length(vecToLight);
        if (distance > pointLights[i].range) continue;
        
        vecToLight = normalize(vecToLight);
        
        float spot = pow(max(dot(-vecToLight, pointLights[i].directionAndAngle.xyz), 1e-5f), pointLights[i].directionAndAngle.w);
        float3 attenuateConstants = float3(pointLights[i].aConstant, pointLights[i].aLinear, pointLights[i].aQuadratic);
        float attenuate = spot / dot(attenuateConstants, float3(1.0f, distance, distance * distance));
        
        float diffuseFactor = saturate(dot(worldNormal, vecToLight));
        float4 diffuseColor = pointLights[i].color * diffuseFactor * attenuate;
        
        input.light += diffuseColor;
    }
    //input.light.a = 0.0f; // Funny effect // could be useful
    
    float4 returnColor = lerp(texColor * input.light, fogColor, fogFactor);
    
    return returnColor;
}