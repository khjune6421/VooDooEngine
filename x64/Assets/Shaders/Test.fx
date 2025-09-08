cbuffer ConstBuffer
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
}

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
};

VSOutput VSMain(float4 pos : POSITION, float4 col : COLOR0)
{
	VSOutput o = (VSOutput)0;
    pos.w = 1.0f;

    pos = mul(pos, WVP);	 
    //pos = mul(pos, world);
    //pos = mul(pos, view);
    //pos = mul(pos, projection);	 	

	o.pos = pos;
	o.col = col;

    return o;
}

float4 PSMain(float4 pos : SV_POSITION, float4 col : COLOR0) : SV_TARGET
{
    return col;
}