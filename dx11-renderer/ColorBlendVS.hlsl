cbuffer CBuf
{
	matrix model;
    matrix modelViewProjection;
};

struct VSOut
{
	float4 color : Color;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float4 color : Color)
{
	VSOut vso;
    vso.pos = mul(float4(pos, 1.0f), modelViewProjection);
	vso.color = color;
	return vso;
}