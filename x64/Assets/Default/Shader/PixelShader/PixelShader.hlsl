struct PSInput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    
    float PSFloatA : TEXCOORD0;
    float PSFloatB : TEXCOORD1;
    float PSFloatC : TEXCOORD2;
    float PSFloatD : TEXCOORD3;
};

float4 main(PSInput input) : SV_TARGET
{
    return input.col;
}