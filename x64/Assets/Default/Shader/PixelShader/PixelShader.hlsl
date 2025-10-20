Texture2D _MainTex;
SamplerState sampler_MainTex;

struct PointLight
{
    float4 worldPos;
    float4 color; // w is intensity
    float range;
    
    // Attenuation factors
    float aConstant;
    float aLinear;
    float aQuadratic;
};

cbuffer PointLightConstBuffer : register(b0)
{
    PointLight pointLights;
}

struct PSInput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
    
    float4 light : TEXCOORD1;
    float4 posWorld : WORLDPOS0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = _MainTex.Sample(sampler_MainTex, input.uv);
    
    float3 vecToLight = pointLights.worldPos.xyz - input.posWorld.xyz;
    float diffuseFactor = saturate(dot(input.norm, normalize(vecToLight)));
    
    float distance = length(vecToLight);
    float3 attenuateConstants = float3(pointLights.aConstant, pointLights.aLinear, pointLights.aQuadratic);
    float attenuate = 1.0f / dot(attenuateConstants, float3(1.0f, distance, distance * distance));
    
    float4 diffuseColor = pointLights.color * diffuseFactor * attenuate; // This can be optimized further
    
    return texColor * (diffuseColor + input.light);
}