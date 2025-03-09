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

float3 CalcSpecular(float3 specularColor,
                    float3 specularIntensity,
                    float3 viewFragPos,
                    float3 viewLightPos,
                    float3 fragNormal,
                    float specularPower,
                    float att)
{
    // Vector to Light
    const float3 vToL = viewLightPos - viewFragPos;
    
    // I forgot
    const float3 w = fragNormal * dot(vToL, fragNormal);
    const float3 r = normalize(w * 2.0f - vToL);
    
    const float3 specular = att * specularColor * specularIntensity * pow(max(0.0f, dot(-r, normalize(viewFragPos))), specularPower);
    return specular;
}