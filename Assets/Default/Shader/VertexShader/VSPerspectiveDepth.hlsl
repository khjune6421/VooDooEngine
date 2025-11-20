cbuffer MatrixConstBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
    
    matrix normalMatrix;
}

struct VSInput
{
    float4 pos0 : POSITION0;
    float3 norm0 : NORMAL0;
    float2 uv0 : TEXCOORD0;
    float3 tangent0 : TANGENT0;
};

struct VSOutput
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    
    float3 worldPos : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    input.pos0.w = 1.0f;
    output.pos = mul(input.pos0, WVP);
    output.uv = input.uv0;
    output.worldPos = mul(input.pos0, world).xyz;
    
    return output;
}