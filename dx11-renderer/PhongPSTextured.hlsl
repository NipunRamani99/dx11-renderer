#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"

#include "PointLight.hlsl"

cbuffer ObjectData : register(b1)
{
	float specularIntensity = 0.1f;
	float specularPower = 1.0f;
};

cbuffer CamData : register(b2) {
	float3 camPos = {0.0,0.0,0.0};
}

cbuffer CBuf : register(b3)
{
    matrix model : packoffset(c0);
    matrix view : packoffset(c4);
    matrix projection : packoffset(c8);
};

Texture2D diffuseTex : register(t0);
SamplerState texSampler : register(s0);

float4 main(float3 viewPos : Position, float3 n : Normal, float2 texCoord : TexCoord) : SV_Target
{
    float4 texC = diffuseTex.Sample(texSampler, texCoord);
    clip(texC.a < 0.1f ? -1 : 1);

    // Vector to Light
    const float3 vToL = lightPos - viewPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
    n = normalize(n);
	// diffuse attenuation
    const float att = CalcAttenuate(distToL, attLin, attQuad);
	// diffuse intensity
    const float3 diffuse = CalcDiffuse(diffuseColor, dirToL, n, att);
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float3 specular = CalcSpecular(diffuseColor, specularIntensity, viewPos, lightPos, n, specularPower, att);
    const float3 specularReflectionColor = float3(1.0f,1.0f,1.0f);

	// final color
    return float4(saturate((diffuse + ambient) * diffuseTex.Sample(texSampler, texCoord).rgb + specular * specularReflectionColor), texC.a);
}