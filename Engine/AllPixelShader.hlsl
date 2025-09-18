struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    
    float PSFloatA : TEXCOORD0;
    float PSFloatB : TEXCOORD1;
    float PSFloatC : TEXCOORD2;
    float PSFloatD : TEXCOORD3;
};

float4 main(PSInput input) : SV_TARGET
{
    input.col.a = input.PSFloatA;
    input.col.r = input.PSFloatB;
    input.col.g = input.PSFloatC;
    input.col.b = input.PSFloatD;
    
    return input.col;
}