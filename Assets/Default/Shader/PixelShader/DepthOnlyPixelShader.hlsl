SamplerState defaultTexSampler : register(s1);
Texture2D mainTex : register(t1);

cbuffer LightPosBuffer : register(b0)
{
    float4 lightPositionAndRange; // xyz = position, w = range
}

struct PSInput
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    
    float3 worldPos : TEXCOORD1;
};

float main(PSInput input) : SV_DEPTH
{
    float4 texColor = mainTex.Sample(defaultTexSampler, input.uv);
    clip(texColor.a - 0.25f);
    
    float3 lightToPixel = input.worldPos - lightPositionAndRange.xyz;
    float distance = length(lightToPixel);
    
    return distance / lightPositionAndRange.w;
}