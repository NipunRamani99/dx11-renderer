#include "MatrixOps.hlsl"
cbuffer CBuf : register(b0)
{
    matrix model : packoffset(c0);
    matrix view : packoffset(c4);
    matrix projection : packoffset(c8);
};

struct VSIn
{
    float3 pos : Position;
    float3 n : Normal;
    float3 tangent : Tangent;
    float3 bitangent : BiTangent;
    float3 texCoord : TexCoord;
};

struct VSOut
{
    float3 viewPos : Position;
    float3 normal : Normal;
    float3 tangent : Tangent;
    float3 bitangent : BiTangent;
    float2 texCoord : TexCoord;
    float4 pos : SV_Position;
};

VSOut main(VSIn input)
{
    VSOut vso;
    const float4x4 modelView = mul(model, view);
    const float4x4 modelViewProj = mul(modelView, projection);
    float3 N = input.n;
    float3 T = input.tangent;
    float3 B = input.bitangent;
    vso.viewPos = (float3) mul(float4(input.pos, 1.0f), modelView);
    vso.normal = (float3) mul(N, (float3x3) (modelView));
    vso.tangent = (float3) mul(T, (float3x3) (modelView));
    vso.bitangent = (float3) mul(B, (float3x3) (modelView));
    vso.pos = mul(float4(input.pos, 1.0f), modelViewProj);
    vso.texCoord = input.texCoord;
    return vso;
}