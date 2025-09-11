cbuffer ConstBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
    
    float time;
    int dummy[3];
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
    float time : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    
    input.pos.w = 1.0f;
    input.pos = mul(input.pos, WVP);
    
    output.pos = input.pos;
    output.col = input.col;
    
    output.time = time;
    
    return output;
}