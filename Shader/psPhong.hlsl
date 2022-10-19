#include "Header.hlsl"
Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);

cbuffer cbColor: register(b0) {
	float3 surfColor;
}

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float3 lightPos = float3(1000.f, 1000.f, 1000.f);
static const float kd = 0.5, ks = 0.2f, m = 100.0f;

float4 main(PSPhongIn i) : SV_TARGET
{
    float4 texColor = colorMap.Sample(colorSampler, i.tex);
    float3 surfaceColor = lerp(surfColor, texColor.xyz, texColor.a);
    float3 viewVec = normalize(i.viewVec);
    float3 normal = normalize(i.norm);
    float3 color = surfaceColor.rgb * ambientColor; //ambient reflection
    float3 lightVec = normalize(lightPos.xyz - i.worldPos);
    float3 halfVec = normalize(viewVec + lightVec);
    color += lightColor * surfaceColor.xyz * kd * saturate(dot(normal, lightVec)); //diffuse reflection
    color += lightColor * ks * pow(saturate(dot(normal, halfVec)), m); //specular reflection
    return float4(saturate(color), 1.f);
}