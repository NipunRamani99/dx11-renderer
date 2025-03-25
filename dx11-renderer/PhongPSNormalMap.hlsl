#include "MatrixOps.hlsl"
#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"
#include "PointLight.hlsl"

cbuffer ObjectCBuf : register(b4)
{
    float specularIntensity;
    float specularPower;
    bool normalMapEnabled = true;
    float padding[1];
};

cbuffer CamData : register(b2)
{
    float3 camPos = { 0.0, 0.0, 0.0 };
}

#include "Transform.hlsl"

Texture2D diffuseTex : register(t0);
Texture2D normalTex : register(t2);
SamplerState texSampler : register(s0);

float4 main(float3 viewFragPos : Position, float3 normalView : Normal, float3 tangentView : Tangent, float3 bitangentView : BiTangent, float2 texCoord : TexCoord) : SV_Target
{
    float3 texNorm = normalize(normalView);
    
    
    if (normalMapEnabled)
    {
    
        float3 sampleNorm = normalTex.Sample(texSampler, texCoord).xyz;
        normalView = MapNormal(sampleNorm, normalView, tangentView, bitangentView, view);
    }

    float4 texC = diffuseTex.Sample(texSampler, texCoord);
    float3 specularPow = specularPower;

	// fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);

	// diffuse attenuation
    const float att = CalcAttenuate(lv.distToL, attLin, attQuad);

	// diffuse intensity
    const float3 diffuse = CalcDiffuse(diffuseColor, lv.dirToL, normalView, att);

    // calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float3 specular = CalcSpecular(
        diffuseColor, diffuseIntensity, normalView,
        lv.vToL, viewFragPos, att, specularPower
    );
   
    // final color
    return float4(saturate((diffuse + ambient) * texC.rgb ), texC.a);
} 