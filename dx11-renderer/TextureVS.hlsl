cbuffer CBuf
{
	matrix model;
    matrix modelViewProjection;
};

struct VSOut
{
	float2 tex : TexCoord;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float2 tex : TexCoord)
{
	VSOut vso;
    vso.pos = mul(float4(pos, 1.0f), modelViewProjection);
	vso.tex = tex;
	return vso;
}