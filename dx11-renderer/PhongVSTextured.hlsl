cbuffer CBuf : register(b0)
{
	matrix model : packoffset(c0);
	matrix view : packoffset(c4);
	matrix projection : packoffset(c8);
};


struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
    float2 texCoord : TexCoord;
	float4 pos : SV_Position;
};

struct VSIn
{
    float3 pos : Position;
    float3 n : Normal;
    float2 texCoord : TexCoord;
};

VSOut main(VSIn input)
{
	VSOut vso;
	const float4x4 modelView = mul(model, view);
	const float4x4 modelViewProj = mul(modelView, projection);
	vso.worldPos = (float3)mul(float4(input.pos, 1.0f), modelView);
    vso.normal = (float3)mul(input.n, (float3x3)modelView);
	vso.pos = mul(float4(input.pos, 1.0f),  modelViewProj);
	vso.texCoord = input.texCoord;
	return vso;
} 