#include "ShaderOps.hlsl"

cbuffer LightCBuf : register(b0)
{
	float3 lightPos;
    float3 ambient = { 0.05f, 0.05f, 0.05f };
    float3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    float diffuseIntensity = 1.0f;
    float attConst = 1.0f;
    float attLin = 0.045f;
    float attQuad = 0.0075f;
};

cbuffer ObjectData : register(b1)
{
	float3 materialColor = {0.7, 0.7, 0.5};
	float specularIntensity = 0.1f;
	float specularPower = 1.0f;
};

cbuffer CamData : register(b2) {
	float3 camPos = {0.0,0.0,0.0};
}

Texture2D diffuseTex : register(t0);
SamplerState texSampler : register(s0);
Texture2D specularTex : register(t1);

float4 main(float3 viewPos : Position, float3 n : Normal, float2 texCoord : TexCoord) : SV_Target
{
    n = normalize(n);
	// fragment to light vector data 
    const float3 vToL = lightPos - viewPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// diffuse attenuation
    const float att = CalcAttenuate(distToL, attLin, attQuad);
	// diffuse intensity
    const float3 diffuse = CalcDiffuse(diffuseColor, dirToL, n, att);
	
    const float4 specularSample = specularTex.Sample(texSampler, texCoord);
    const float3 specularReflectionColor = specularSample.rgb;
    const float specularPower = pow(2.0f,specularSample.a * 13.0f);
    
    const float3 specular = CalcSpecular(specularReflectionColor, specularIntensity, viewPos, lightPos, n, specularPower, att);
	// final color
    return float4(saturate((diffuse + ambient) * diffuseTex.Sample(texSampler, texCoord).rgb + specular * specularReflectionColor), 1.0f);
}