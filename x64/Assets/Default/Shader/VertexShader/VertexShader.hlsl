cbuffer TestConstBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
    
    matrix normalMatrix;
}

cbuffer AmbientLightConstBuffer : register(b1)
{
    float4 ambientLight;
}

cbuffer DirectionalLightConstBuffer : register(b2) // Just one directional light
{
    float4 dirLightDirection;
    float4 dirLightColor;
}

struct VSInput
{
    float4 pos0 : POSITION0;
    float4 col0 : COLOR0;
    float3 norm0 : NORMAL0;
    float3 tangent0 : TANGENT0;
    float2 uv0 : TEXCOORD0;
};

struct VSOutput
{
    float4 pos : SV_POSITION0;
    float4 posWorld : WORLDPOS0;
    
    float4 col : COLOR0;
    float4 light : COLOR1;
    
    float3 norm : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BITANGENT0;
    
    float2 uv : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    input.pos0.w = 1.0f;
    
    output.pos = mul(input.pos0, WVP);
    output.posWorld = mul(input.pos0, world);
    
    output.col = input.col0;
    output.norm = normalize(mul(float4(input.norm0, 0.0f), normalMatrix).xyz); // Inverse scale matrix
    float4 diffuseColor = dirLightColor * saturate(dot(output.norm, -dirLightDirection.xyz));
    output.light = ambientLight + diffuseColor;
    
    output.tangent = normalize(mul(float4(input.tangent0, 0.0f), normalMatrix).xyz);
    output.bitangent = normalize(cross(output.norm, output.tangent));
    
    output.uv = input.uv0;
    
    return output;
}