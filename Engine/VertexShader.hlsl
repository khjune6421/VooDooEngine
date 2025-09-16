cbuffer TestConstBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
    
    float sinTime;
    float cosTime;
    float negsinTime;
    float negcosTime;
}

struct VSInput
{
    float4 pos : POSITION;
    float4 col : COLOR0;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    
    float sinTime : TEXCOORD0;
    float cosTime : TEXCOORD1;
    float negsinTime : TEXCOORD2;
    float negcosTime : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    input.pos.w = 1.0f;
    input.pos = mul(input.pos, WVP);
    
    output.pos = input.pos;
    output.col = input.col;
    
    output.sinTime = sinTime;
    output.cosTime = cosTime;
    output.negsinTime = negsinTime;
    output.negcosTime = negcosTime;
    
    return output;
}