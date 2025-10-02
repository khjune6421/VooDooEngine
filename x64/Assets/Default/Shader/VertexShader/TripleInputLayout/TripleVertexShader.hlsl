cbuffer TestConstBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
    
    float VSFloatA;
    float VSFloatB;
    float VSFloatC;
    float VSFloatD;
}

struct VSInput
{
    float4 pos0 : POSITION0;
    float4 col0 : COLOR0;
    float2 uv0 : TEXCOORD0;
    float3 norm0 : NORMAL0;
    
    float4 pos1 : POSITION1;
    float4 col1 : COLOR1;
    float2 uv1 : TEXCOORD1;
    float3 norm1 : NORMAL1;
    
    float4 pos2 : POSITION2;
    float4 col2 : COLOR2;
    float2 uv2 : TEXCOORD2;
    float3 norm2 : NORMAL2;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    
    float PSFloatA : TEXCOORD0;
    float PSFloatB : TEXCOORD1;
    float PSFloatC : TEXCOORD2;
    float PSFloatD : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float4 outPos0 = lerp(input.pos1, input.pos2, VSFloatA);
    float4 outPos = lerp(input.pos0, outPos0, VSFloatB);
    
    outPos.w = 1.0f;
    outPos = mul(outPos, WVP);
    
    output.pos = outPos;
    output.col = input.col0;
    
    output.PSFloatA = VSFloatA;
    output.PSFloatB = VSFloatB;
    output.PSFloatC = VSFloatC;
    output.PSFloatD = VSFloatD;
    
    return output;
}