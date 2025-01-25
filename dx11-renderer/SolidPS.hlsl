cbuffer ObjectData : register(b0)
{
	float4 color = {1.0f, 1.0f, 1.0f, 1.0f};
}
float4 main() : SV_TARGET
{
	return color;
}