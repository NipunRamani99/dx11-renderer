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
    bool renderNormals = false;
};

cbuffer ObjectCBuf : register(b4)
{
    float specularIntensity = 0.1f;
    float specularPower = 1.0f;
    bool normalMapEnabled = true;
    bool hasSpecularMap = true;
    bool hasGloss = false;
    float3 specularColor = float3(1.0f, 0.0f, 1.0f);
    float specularMapWeight = 0.671f;
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
Texture2D specTex : register(t1);
Texture2D normalTex : register(t2);
SamplerState texSampler : register(s0);

float4 main(float3 viewPos : Position, float3 normalView : Normal, float3 tangentView : Tangent, float3 bitangentView : BiTangent, float2 texCoord : TexCoord) : SV_Target
{
    float3 texNorm = normalize(normalView);
    float4 texC = diffuseTex.Sample(texSampler, texCoord);
#ifdef MASK
    clip(texC.a < 0.1f ? -1 : 1);
    if(dot(normalView, viewPos) >= 0.0f)
    {
        normalView = -normalView;
    }
#endif
    if (normalMapEnabled)
    {
        float3 sampleNorm = normalTex.Sample(texSampler, texCoord).xyz;
        texNorm = MapNormal(sampleNorm, normalView, tangentView, bitangentView, view);
    }
    float3 specularReflectionColor;
    float3 specularPow = specularPower;
    if(hasSpecularMap)
    {
        const float4 specularSample = specTex.Sample(texSampler, texCoord);
        specularReflectionColor = specularSample.rgb * specularMapWeight;
        if(hasGloss)
        {
            specularPow = pow(2.0f, specularSample.a * 13.0f);
        }
    }
    else
    {
        specularReflectionColor = specularColor;
    }
	// fragment to light vector data 
    const float3 vToL = viewLightPos - viewPos;
    const float distToL = length(vToL);
    const float3 dirToL = vToL / distToL;
	// diffuse attenuation
    const float att = CalcAttenuate(distToL, attLin, attQuad);

	// diffuse intensity
    const float3 diffuse = CalcDiffuse(diffuseColor, dirToL, texNorm, att);
	
    // calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float3 specular = CalcSpecular(specularReflectionColor, specularIntensity, viewPos, viewLightPos, texNorm, specularPower, att);
    
   
    // final color
    return float4(saturate((diffuse + ambient) * texC.rgb + specular), texC.a);
} 