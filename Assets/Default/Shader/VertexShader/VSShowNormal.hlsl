struct VS_INPUT
{
    float3 position0 : POSITION0;
    float3 normal0 : NORMAL0;
    float2 uv0 : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.position = float4(input.position0, 1.0f);
    output.normal = input.normal0;
    output.uv = input.uv0;
    
    return output;
}