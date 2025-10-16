Texture2D _MainTex;
SamplerState sampler_MainTex;

cbuffer AmbientLightConstBuffer : register(b0)
{
    float4 ambientColor;
}

struct PSInput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 diffuse = input.col * input.light;
    float4 texColor = _MainTex.Sample(sampler_MainTex, input.uv);
    
    return texColor * diffuse;
}