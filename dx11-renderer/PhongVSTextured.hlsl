cbuffer CBuf
{
	matrix model;
	matrix modelView;
	matrix modelViewProj;
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
	vso.worldPos = (float3)mul(float4(input.pos, 1.0f), model);
	vso.normal = (float3)mul(input.n, (float3x3)model);
	vso.pos = mul(float4(input.pos, 1.0f), modelViewProj);
	vso.texCoord = input.texCoord;
	return vso;
} 