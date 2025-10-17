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
    
    float3 vecToLight = normalize(pointLights.worldPos.xyz - input.posWorld.xyz);
    float diffuseFactor = saturate(dot(normalize(input.norm), vecToLight));
    float4 diffuseColor = pointLights.color * diffuseFactor;
    
    return texColor * (diffuseColor + ambientColor);
}