cbuffer MatrixBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix WVP;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct GS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

[maxvertexcount(2)]
void main(point VS_OUTPUT input[1], inout LineStream<GS_OUTPUT> outputStream)
{
    GS_OUTPUT output;
    
    float3 worldNormal = normalize(mul(float4(input[0].normal, 0.0f), world).xyz); // this does not work if scale is applied to certain axis
    
    float4 worldPos = mul(float4(input[0].position.xyz, 1.0f), world);
    
    // Start point
    output.position = mul(worldPos, mul(view, projection));
    output.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
    outputStream.Append(output);
    
    // End point
    float4 normalEndPos = worldPos + float4(worldNormal * 1.0f, 0.0f);
    output.position = mul(normalEndPos, mul(view, projection));
    output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    outputStream.Append(output);
    
    outputStream.RestartStrip();
}