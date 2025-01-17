#include "MatrixOps.hlsl"
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

cbuffer CBuf : register(b3)
{
    matrix model : packoffset(c0);
    matrix view : packoffset(c4);
    matrix projection : packoffset(c8);
};

Texture2D diffuseTex : register(t1);
Texture2D normalTex : register(t2);
SamplerState texSampler : register(s0);


float3 TransformNormalToViewSpace(float3 normal, matrix modelMatrix, matrix viewMatrix)
{
    // Combine the model and view matrix
    float4x4 modelViewMatrix = mul(modelMatrix, viewMatrix);

    // Compute the normal matrix (transpose of the inverse)
    float3x3 normalMatrix = inverse(linearPart(modelViewMatrix));

    // Transform the normal into view space
    return normalize(mul(normal, normalMatrix));
}

float4 main(float3 viewPos : Position, float3 n : Normal, float2 texCoord : TexCoord) : SV_Target
{
    float3 texNorm = normalTex.Sample(texSampler, texCoord).xyz;
    texNorm.x = texNorm.x * 2.0f - 1.0f;
    texNorm.y = -texNorm.y * 2.0f + 1.0f;
    texNorm.z = -texNorm.z;
    //n.xy = 2.0f * texNorm.xy - 1.0f;

    n = TransformNormalToViewSpace(texNorm, model, view);
    
	// fragment to light vector data 
    const float3 vToL = lightPos - viewPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	
    // diffuse attenuation
    const float att = CalcAttenuate(distToL, attLin, attQuad);
    
	// diffuse intensity
    const float3 diffuse = CalcDiffuse(diffuseColor, dirToL, n, att);
	
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float3 specular = CalcSpecular(diffuseColor, specularIntensity, viewPos, lightPos, n, specularPower, att);
    const float3 specularReflectionColor = float3(1.0f,1.0f,1.0f);

	// final color
    return float4(saturate((diffuse + ambient) * diffuseTex.Sample(texSampler, texCoord).rgb + specular * specularReflectionColor), 1.0f);

}