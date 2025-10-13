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

cbuffer LightConstBuffer : register(b1)
{
    float4 localPosition;
    float4 ambientColor;
    float4 diffuseColor;
    
    float range;
    float intensity;
    float attenuation;
    float padding;
}

struct VSInput
{
    float4 pos0 : POSITION0;
    float4 col0 : COLOR0;
    float3 norm0 : NORMAL0;
    float2 uv0 : TEXCOORD0;
    
    float4 pos1 : POSITION1;
    float4 col1 : COLOR1;
    float3 norm1 : NORMAL1;
    float2 uv1 : TEXCOORD1;
    
    float4 pos2 : POSITION2;
    float4 col2 : COLOR2;
    float3 norm2 : NORMAL2;
    float2 uv2 : TEXCOORD2;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    
    float4 light : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float4 lerpPos0 = lerp(input.pos1, input.pos2, VSFloatA);
    float4 lerpPos1 = lerp(input.pos0, lerpPos0, VSFloatB);
    
    float3 lerpNorm0 = lerp(input.norm1, input.norm2, VSFloatA);
    float3 lerpNorm1 = lerp(input.norm0, lerpNorm0, VSFloatB);
    
    lerpPos1.w = 1.0f;
    output.pos = mul(lerpPos1, WVP);
    
    float3 localVertexPos = lerpPos1.xyz;
    float3 toLight = localPosition.xyz - localVertexPos;
    float distance = length(toLight);
    
    float3 lightDir = normalize(toLight);
    float3 normal = normalize(lerpNorm1);
    float diffuseFactor = max(dot(normal, lightDir), 0.0f);
    
    float attenuationFactor = 1.0f;
    if (distance > 0.0f) attenuationFactor = 1.0f / (1.0f + attenuation * distance * distance);
    if (distance > range) attenuationFactor = 0.0f;
    
    output.light = diffuseColor * diffuseFactor * attenuationFactor * intensity + ambientColor;
    
    output.col = input.col0;
    
    return output;
}