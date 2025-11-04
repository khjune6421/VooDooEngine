SamplerState defaultTexSampler : register(s0);
Texture2D mainTex : register(t1);

struct PSInput
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
};

void main(PSInput input)
{
    float4 texColor = mainTex.Sample(defaultTexSampler, input.uv);
    clip(texColor.a - 0.25f);
}