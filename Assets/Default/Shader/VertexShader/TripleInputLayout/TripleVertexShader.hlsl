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
    float padding;
}

cbuffer AnimationConstBuffer : register(b2)
{
    int currentShapeIndex;
    int nextShapeIndex;
    float interpolationFactor;

    float padding0;
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
    
    float4 currentPos;
    float4 nextPos;
    float3 currentNorm;
    float3 nextNorm;
    
    if (currentShapeIndex == 0)
    {
        currentPos = input.pos0;
        currentNorm = input.norm0;
    }
    else if (currentShapeIndex == 1)
    {
        currentPos = input.pos1;
        currentNorm = input.norm1;
    }
    else if (currentShapeIndex == 2)
    {
        currentPos = input.pos2;
        currentNorm = input.norm2;
    }
    
    if (nextShapeIndex == 0)
    {
        nextPos = input.pos0;
        nextNorm = input.norm0;
    }
    else if (nextShapeIndex == 1)
    {
        nextPos = input.pos1;
        nextNorm = input.norm1;
    }
    else if (nextShapeIndex == 2)
    {
        nextPos = input.pos2;
        nextNorm = input.norm2;
    }
    
    float4 lerpPos = lerp(currentPos, nextPos, interpolationFactor);
    float3 lerpNorm = lerp(currentNorm, nextNorm, interpolationFactor);
    
    lerpPos.w = 1.0f;
    output.pos = mul(lerpPos, WVP);
    
    float3 localVertexPos = lerpPos.xyz;
    float3 toLight = localPosition.xyz - localVertexPos;
    float distance = length(toLight);
    
    float3 lightDir = normalize(toLight);
    float3 normal = normalize(lerpNorm);
    float diffuseFactor = max(dot(normal, lightDir), 0.0f);
    
    float attenuationFactor = 1.0f;
    if (distance > 0.0f) attenuationFactor = 1.0f / (1.0f + attenuation * distance * distance);
    if (distance > range) attenuationFactor = 0.0f;
    
    output.light = diffuseColor * diffuseFactor * attenuationFactor * intensity + ambientColor;
    
    output.col = input.col0;
    
    return output;
}