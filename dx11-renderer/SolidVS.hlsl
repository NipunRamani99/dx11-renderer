cbuffer CBuf : register(b0)
{
	matrix model : packoffset(c0);
	matrix view : packoffset(c4);
	matrix projection : packoffset(c8);
};

float4 main(float3 pos : Position) : SV_Position
{
	const float4x4 modelView = mul(model, view);
	const float4x4 modelViewProj = mul(modelView, projection);
	return mul(float4(pos, 1.0f), modelViewProj);
}