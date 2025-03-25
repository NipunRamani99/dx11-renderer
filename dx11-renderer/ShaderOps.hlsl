float3 MapNormal(float3 normalSample, 
                 float3 normalWorld, 
                 float3 tangentWorld, 
                 float3 bitangentWorld, 
                 matrix viewMatrix)
{
    float3 tanNormal = normalSample * 2.0f - 1.0f;
    float3x3 TBN = float3x3(normalize(tangentWorld), normalize(bitangentWorld), normalize(normalWorld));
    float3 normal = normalize(mul(tanNormal, TBN));
    return normalize(normal);
}
 
float CalcAttenuate(float distToL,
                    float attLin,
                    float attQuad)
{
    return 1.0f / (1.0f + attLin * distToL + attQuad * distToL * distToL);
}

float3 CalcDiffuse(float3 diffuseColor,
                   float3 dirLight,
                   float3 normal,
                   float att)
{
    return diffuseColor * 1.0f * att * max(0.0f, dot(dirLight, normal));
}

float3 CalcSpecular(
    const in float3 specularColor,
    uniform float specularIntensity,
    const in float3 viewNormal,
    const in float3 viewFragToL,
    const in float3 viewPos,
    const in float att,
    const in float specularPower)
{
    // calculate reflected light vector
    const float3 w = viewNormal * dot(viewFragToL, viewNormal);
    const float3 r = normalize(w * 2.0f - viewFragToL);
    // vector from camera to fragment (in view space)
    const float3 viewCamToFrag = normalize(viewPos);
    // calculate specular component color based on angle between
    // viewing vector and reflection vector, narrow with power function
    return att * specularColor * specularIntensity * pow(max(0.0f, dot(-r, viewCamToFrag)), specularPower);
}