cbuffer TestConstBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
}

cbuffer LightConstBuffer : register(b1)
{
    float4 localPosition;
    float4 ambientColor;
    float4 diffuseColor;
    
    float range;
    float intensity;
    float attenuation;
    int isBackfaceLighting;
}

struct VSInput
{
    float4 pos0 : POSITION0;
    float4 col0 : COLOR0;
    float3 norm0 : NORMAL0;
    float2 uv0 : TEXCOORD0;
};

struct VSOutput
{
    float4 pos : SV_POSITION0;
    float4 col : COLOR0;
    
    float4 light : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    input.pos0.w = 1.0f;
    output.pos = mul(input.pos0, WVP);
    
    float3 localVertexPos = input.pos0.xyz;
    float3 toLight = localPosition.xyz - localVertexPos;
    float distance = length(toLight);
    
    float3 lightDir = normalize(toLight);
    float3 normal = normalize(input.norm0);
    
    float diffuseFactor;
    if (isBackfaceLighting)
        diffuseFactor = abs(dot(normal, lightDir));
    else
        diffuseFactor = max(dot(normal, lightDir), 0.0f);
    
    float attenuationFactor = 1.0f;
    if (distance > 0.0f) attenuationFactor = 1.0f / (1.0f + attenuation * distance * distance); // + 1.0f to prevent div by 0 // TODO: change this
    if (distance > range) attenuationFactor = 0.0f;
    
    output.light = diffuseColor * diffuseFactor * attenuationFactor * intensity + ambientColor;
    
    output.col = input.col0;
    
    return output;
}