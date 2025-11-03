cbuffer MatrixConstBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
    
    matrix normalMatrix;
}

cbuffer CameraConstBuffer : register(b1)
{
    float4 cameraPos;
}

cbuffer AmbientLightConstBuffer : register(b2)
{
    float4 ambientLight;
}

cbuffer DirectionalLightConstBuffer : register(b3) // Just one directional light
{
    float4 directionalLightNormal;
    float4 directionalLightColor;
}

cbuffer ShadowConstBuffer : register(b4)
{
    matrix lightView;
    matrix lightProjection;
    matrix lightWVP;
}

struct VSInput
{
    float4 pos0 : POSITION0;
    float3 norm0 : NORMAL0;
    float2 uv0 : TEXCOORD0;
    float3 tangent0 : TANGENT0;
};

struct VSOutput
{
    float4 pos : SV_POSITION0;
    float3 norm : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
    
    float4 posWorld : WORLDPOS0;
    float4 light : COLOR1;
    float3 bitangent : BITANGENT0;
    
    float4 lightSpacePos : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    input.pos0.w = 1.0f;
    
    output.pos = mul(input.pos0, WVP);
    output.norm = normalize(mul(float4(input.norm0, 0.0f), normalMatrix).xyz); // Inverse scale matrix
    output.uv = input.uv0;
    output.tangent = normalize(mul(float4(input.tangent0, 0.0f), normalMatrix).xyz);
    
    output.posWorld = mul(input.pos0, world);
    
    float diffuseFactor = dot(output.norm, -directionalLightNormal.xyz);
    output.light = ambientLight + directionalLightColor * diffuseFactor;
    
    output.bitangent = normalize(cross(output.norm, output.tangent));
    
    if (diffuseFactor > 0.0f)
    {
        float3 viewDirection = normalize(cameraPos.xyz - output.posWorld.xyz);
        float3 blinnPhongHalfVector = normalize(-directionalLightNormal.xyz + viewDirection);

        float specularFactor = dot(output.norm, blinnPhongHalfVector);
        output.light += directionalLightColor * specularFactor;
    }
    
    output.lightSpacePos = mul(input.pos0, lightWVP);
    
    return output;
}