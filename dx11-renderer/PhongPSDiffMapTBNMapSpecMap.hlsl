#include "MatrixOps.hlsl"
#include "ShaderOps.hlsl"

cbuffer LightCBuf : register(b0)
{
    float3 viewLightPos;
    float3 ambient = { 0.05f, 0.05f, 0.05f };
    float3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    float diffuseIntensity = 1.0f;
    float attConst = 1.0f;
    float attLin = 0.045f;
    float attQuad = 0.0075f;
};

cbuffer ObjectData : register(b1)
{
    float3 materialColor = { 0.7, 0.7, 0.5 };
    float specularIntensity = 0.1f;
    float specularPower = 1.0f;
};

cbuffer NormalData : register(b4)
{
    bool normalMapEnabled = false;
    bool negateXAndY = false;
};

cbuffer CamData : register(b2)
{
    float3 camPos = { 0.0, 0.0, 0.0 };
}

cbuffer CBuf : register(b3)
{
    matrix model : packoffset(c0);
    matrix view : packoffset(c4);
    matrix projection : packoffset(c8);
};

Texture2D diffuseTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D specTex : register(t2);

SamplerState texSampler : register(s1);

float4 main(float3 viewPos : Position, float3 normalView : Normal, float3 tangentView : Tangent, float3 bitangentView : BiTangent, float2 texCoord : TexCoord) : SV_Target
{
    float3 sampleNorm = normalTex.Sample(texSampler, texCoord).xyz;
    
    if (negateXAndY)
    {
        sampleNorm.x = sampleNorm.x * 2.0f - 1.0f;
        sampleNorm.y = -sampleNorm.y * 2.0f + 1.0f;
        sampleNorm.z = -sampleNorm.z * 2.0f + 1.0f;
    }
    else
    {
        sampleNorm.x = sampleNorm.x * 2.0f - 1.0f;
        sampleNorm.y = sampleNorm.y * 2.0f - 1.0f;
        sampleNorm.z = sampleNorm.z * 2.0f - 1.0f;
    }
    float3 texNorm = MapNormal(sampleNorm, normalView, tangentView, bitangentView, view);
    
	// fragment to light vector data 
    const float3 vToL = viewLightPos - viewPos;
    const float distToL = length(vToL);
    const float3 dirToL = vToL / distToL;
	// diffuse attenuation
    const float att = CalcAttenuate(distToL, attLin, attQuad);
    
	// diffuse intensity
    const float3 diffuse = CalcDiffuse(diffuseColor, dirToL, texNorm, att);
	
    // calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float3 specular = CalcSpecular(diffuseColor, specularIntensity, viewPos, viewLightPos, texNorm, specularPower, att);
    
    const float3 specularReflectionColor = float3(1.0f, 1.0f, 1.0f);

	// final color
    return float4(saturate((diffuse + ambient) * diffuseTex.Sample(texSampler, texCoord).rgb + specular * specularReflectionColor), 1.0f);
}