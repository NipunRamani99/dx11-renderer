struct VS_OUTPUT
{
	float3 color : Color;
	float4 pos : SV_POSITION;
};

cbuffer CBuf{
	matrix transform;
}
VS_OUTPUT main(float2 pos : Position, float3 color: Color)
{
	VS_OUTPUT vs_out;
	vs_out.pos = float4(pos.x, pos.y, 0.0f, 1.0f);
	vs_out.color = color;
	vs_out.pos = mul(vs_out.pos, transform);
	return vs_out;
}