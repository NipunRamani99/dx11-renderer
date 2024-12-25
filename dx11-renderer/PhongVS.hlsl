cbuffer CBuf
{
	matrix model;
	matrix view;
	matrix projection;
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
    vso.worldPos = (float3) mul(float4(input.pos, 1.0f), mul(model, view));
    vso.normal = (float3) mul(input.n, (float3x3) mul(model, view));
    vso.pos = mul(float4(input.pos, 1.0f), mul(model, mul(view, projection)));
	return vso;
}