struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float time : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    input.col.a *= cos(input.time);
    input.col.r *= sin(input.time);
    input.col.g *= -cos(input.time);
    input.col.b *= -sin(input.time);
    
    return input.col;
}