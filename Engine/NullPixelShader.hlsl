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
    input.col.a = 1.0f;
    input.col.r = 0.0f;
    input.col.g = 0.0f;
    input.col.b = 0.0f;
    
    return input.col;
}