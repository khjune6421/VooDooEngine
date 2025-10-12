struct PSInput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    
    float4 light : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    return input.col * (input.light);
}