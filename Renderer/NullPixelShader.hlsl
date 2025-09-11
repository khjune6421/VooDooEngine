struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    
    float sinTime : TEXCOORD0;
    float cosTime : TEXCOORD1;
    float negsinTime : TEXCOORD2;
    float negcosTime : TEXCOORD3;
};

float4 main(PSInput input) : SV_TARGET
{
    return input.col;
}