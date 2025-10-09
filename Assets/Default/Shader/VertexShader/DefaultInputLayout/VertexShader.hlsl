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
    float4 pos : POSITION0;
    float4 col : COLOR0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    
    float PSFloatA : TEXCOORD0;
    float PSFloatB : TEXCOORD1;
    float PSFloatC : TEXCOORD2;
    float PSFloatD : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    input.pos.w = 1.0f;
    input.pos = mul(input.pos, WVP);
    
    //input.norm = mul((float3x3) world, input.norm); // Later change it so that only light get calculated with inverse transpose world matrix
    //input.norm = normalize(input.norm);
    
    output.pos = input.pos;
    output.col = input.col;
    
    output.PSFloatA = VSFloatA;
    output.PSFloatB = VSFloatB;
    output.PSFloatC = VSFloatC;
    output.PSFloatD = VSFloatD;
    
    return output;
}