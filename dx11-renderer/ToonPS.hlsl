cbuffer LightCBuf : register(b0)
{
    float3 lightPos;
    float3 ambient = { 0.05f, 0.05f, 0.05f };
    float3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    float3 LightDirection = { -1.0f, 0.0f, 0.0f };
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

// Output structure from the vertex shader to the pixel shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : Normal;
};

float4 main(PS_INPUT input) : SV_Target
{
    // Normalize the normal vector
    float3 normal = normalize(input.Normal);

    // Normalize light direction (assuming it's in world space)
    float3 lightDir = normalize(-LightDirection);

    // Compute the dot product between the normal and light direction
    float NdotL = dot(normal, lightDir);

  
    // Compute the final color by multiplying with the diffuse color
    float4 color = float4(diffuseColor, 1.0) * NdotL;

    return color;
}