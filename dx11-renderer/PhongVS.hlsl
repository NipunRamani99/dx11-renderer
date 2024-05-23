cbuffer CBuf
{
	matrix model;
	matrix modelViewProj;
};

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float4 pos : SV_Position;
};

struct VSIn
{
    float3 pos : Position;
    float3 n : Normal;
};

VSOut main(VSIn input)
{
	VSOut vso;
	vso.worldPos = (float3)mul(float4(input.pos, 1.0f), model);
	vso.normal = mul(input.n, (float3x3)model);
	vso.pos = mul(float4(input.pos, 1.0f), modelViewProj);
	return vso;
}