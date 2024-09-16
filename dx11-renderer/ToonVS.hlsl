// Constant buffer for transformation matrices and lighting parameters
/* cbuffer ConstantBuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float3   LightDirection; // Direction towards the light source in world space
    float4   DiffuseColor;
};
*/

cbuffer ConstantBuffer : register(b0)
{
    matrix modelView;
    matrix modelViewProj;
};


// Input structure from the vertex buffer
struct VSIn
{
    float3 pos : Position;
    float3 n   : Normal;
};

// Output structure from the vertex shader to the pixel shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : Normal;
};

PS_INPUT main(VSIn vsIn)
{
    float4 out_pos = mul(float4(vsIn.pos, 1.0f), modelViewProj);
    PS_INPUT psIn;
    psIn.Position = out_pos;
    psIn.Normal = normalize(mul(vsIn.n, (float3x3)modelView));
    return psIn;
}