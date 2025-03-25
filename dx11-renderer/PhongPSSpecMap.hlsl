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

cbuffer ObjectCBuf : register(b1)
{
	float specularPower = 1.0f;
    float hasGloss = false;
    float specularMapWeight;
};

cbuffer CamData : register(b2) {
	float3 camPos = {0.0,0.0,0.0};
}

Texture2D diffuseTex : register(t0);
Texture2D specularTex : register(t1);
SamplerState texSampler : register(s0);


float4 main(float3 viewPos : Position, float3 n : Normal, float2 texCoord : TexCoord) : SV_Target
{
    float4 texC = diffuseTex.Sample(texSampler, texCoord);
    clip(texC.a < 0.1f ? -1 : 1);
    n = normalize(n);
	// fragment to light vector data 
    const float3 vToL = lightPos - viewPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// diffuse attenuation
    const float att = CalcAttenuate(distToL, attLin, attQuad);
	// diffuse intensity
    const float3 diffuse = CalcDiffuse(diffuseColor, dirToL, n, att);
	float specularPowerLoaded = specularPower;
    const float4 specularSample = specularTex.Sample(texSampler, texCoord);
    if(hasGloss)
    {
        specularPowerLoaded = pow(2.0f,specularSample.a * 13.0f);
    }
    const float3 specularReflectionColor = specularSample.rgb * specularMapWeight;
    
    
    const float3 specular = CalcSpecular(specularReflectionColor, 1.0f, viewPos, lightPos, n, specularPowerLoaded, att);

	// final color
    return float4(saturate((diffuse + ambient) * diffuseTex.Sample(texSampler, texCoord).rgb + specular * specularReflectionColor), texC.a);
}