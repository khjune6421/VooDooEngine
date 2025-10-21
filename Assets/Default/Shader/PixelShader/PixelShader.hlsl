Texture2D _MainTex;
SamplerState sampler_MainTex;

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

cbuffer PointLightConstBuffer : register(b0)
{
    PointLight pointLights[2];
}

struct PSInput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
    
    float4 light : COLOR1;
    float4 posWorld : WORLDPOS0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = _MainTex.Sample(sampler_MainTex, input.uv);
    
    for (int i = 0; i < 2; i++)
    {
        float3 vecToLight = pointLights[i].worldPos.xyz - input.posWorld.xyz;
        
        float distance = length(vecToLight);
        vecToLight = normalize(vecToLight);
        
        float spot = pow(max(dot(-vecToLight, pointLights[i].directionAndAngle.xyz), 1e-5), pointLights[i].directionAndAngle.w);
        float3 attenuateConstants = float3(pointLights[i].aConstant, pointLights[i].aLinear, pointLights[i].aQuadratic);
        float attenuate = spot / dot(attenuateConstants, float3(1.0f, distance, distance * distance));
        
        float diffuseFactor = saturate(dot(input.norm, vecToLight));
        float4 diffuseColor = pointLights[i].color * diffuseFactor * attenuate; // This could be optimized further
        
        input.light += diffuseColor;
    }
    
    return texColor * input.light;
}