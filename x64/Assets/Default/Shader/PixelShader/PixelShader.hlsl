Texture2D _MainTex;
SamplerState sampler_MainTex;

struct PSInput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    float4 light : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 diffuse = input.col * input.light;
    float4 texColor = _MainTex.Sample(sampler_MainTex, input.uv);
    
    return texColor * diffuse;
}