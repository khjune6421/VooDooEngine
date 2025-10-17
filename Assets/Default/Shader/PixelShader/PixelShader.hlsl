Texture2D _MainTex;
SamplerState sampler_MainTex;

cbuffer AmbientLightConstBuffer : register(b0)
{
    float4 ambientColor;
}

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

cbuffer PointLightConstBuffer : register(b1)
{
    PointLight pointLights;
}

struct PSInput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
    
    float4 posWorld : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = _MainTex.Sample(sampler_MainTex, input.uv);
    
    float3 vecToLight = pointLights.worldPos.xyz - input.posWorld.xyz;
    float3 norm = normalize(input.norm);
    float diffuseFactor = saturate(dot(norm, normalize(vecToLight)));
    
    float distance = length(vecToLight);
    float3 attenuateConstants = float3(pointLights.aConstant, pointLights.aLinear, pointLights.aQuadratic);
    float attenuate = 1.0f / dot(attenuateConstants, float3(1.0f, distance, distance * distance));
    
    float4 diffuseColor = pointLights.color * diffuseFactor * attenuate * pointLights.color.w;
    
    return texColor * (diffuseColor + ambientColor);
}