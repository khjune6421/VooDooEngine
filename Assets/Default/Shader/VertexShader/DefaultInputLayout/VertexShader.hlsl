cbuffer TestConstBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
}

struct VSInput
{
    float4 pos0 : POSITION0;
    float4 col0 : COLOR0;
    float3 norm0 : NORMAL0;
    float2 uv0 : TEXCOORD0;
};

struct VSOutput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
    
    float4 posWorld : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    input.pos0.w = 1.0f;
    
    output.pos = mul(input.pos0, WVP);
    output.col = input.col0;
    output.norm = mul(float4(input.norm0, 0.0f), world).xyz;
    output.uv = input.uv0;
    
    output.posWorld = mul(input.pos0, world);
    
    return output;
}