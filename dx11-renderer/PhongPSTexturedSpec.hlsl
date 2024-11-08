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

Texture2D diffuseTex : register(t1);
SamplerState texSampler : register(s1);
Texture2D specularTex : register(t2);

float4 main(float3 worldPos : Position, float3 n : Normal, float2 texCoord : TexCoord) : SV_Target
{
	// fragment to light vector data 
	const float3 vToL =  lightPos - worldPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// diffuse attenuation
    const float att = 1.0f / (1.0f + attLin * distToL + attQuad * (distToL * distToL));
	// diffuse intensity
	float NdotL = max(0.0f, dot(dirToL, normalize(n)));   // Normalized normal
	const float3 diffuse = diffuseColor * att * NdotL;
	// reflected light vector
	const float3 w = normalize(n) * dot( vToL, normalize(n) );
	const float3 r = w * 2.0f - vToL;
    const float4 specularSample = specularTex.Sample(texSampler, texCoord);
    const float3 specularReflectionColor = specularSample.rgb;
    const float specularPower = pow(2.0f,specularSample.a * 13.0f);
    const float3 specular = att * (diffuseColor) * pow(max(0.0f, dot(normalize(-r), normalize(worldPos))), specularPower);
	// final color
    return float4(saturate((diffuse + ambient) * diffuseTex.Sample(texSampler, texCoord).rgb + specular * specularReflectionColor), 1.0f);
}