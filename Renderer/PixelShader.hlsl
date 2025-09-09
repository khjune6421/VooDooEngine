struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float time : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    return input.col;
}