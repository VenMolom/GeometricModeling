#include "Header.hlsl"
Texture2D colorMap : register(t0);
Texture2D normMap : register(t1);
SamplerState texSampler : register(s0);

cbuffer cbColor: register(b0) {
	float3 surfColor;
}

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float3 lightPos = float3(100.f, 100.f, 100.f);
static const float kd = 0.5, ks = 0.2f, m = 100.0f;

float3 normalMapping(float3 N, float3 T, float3 tn)
{
    float3 B = normalize(cross(N, T));
    T = normalize(cross(B, N));
    return mul(transpose(float3x3(T, B, N)), tn);
}

float4 main(PSPhongIn i) : SV_TARGET
{
    float3 dPdx = ddx(i.localPos);
    float3 dPdy = ddy(i.localPos);
    float2 dtdx = ddx(i.tex);
    float2 dtdy = ddy(i.tex);
    float3 T = -normalize(-dPdx * dtdy.y + dPdy * dtdx.y);

    float4 normMapped = normMap.Sample(texSampler, i.tex);
    float3 tn = normMapped.xyz;
    tn = mad(float3(2.0f, -2.0f, 2.0f), tn, float3(-1.0f, 1.0f, -1.0f));

    float3 norm = lerp(i.norm, normalMapping(i.norm, T, tn), normMapped.a * dot(i.norm, float3(0, 1, 0)));

    float4 texColor = colorMap.Sample(texSampler, i.tex);
    float3 surfaceColor = lerp(surfColor, texColor.xyz, texColor.a);
    float3 viewVec = normalize(i.viewVec);
    float3 normal = normalize(norm);
    float3 color = surfaceColor.rgb * ambientColor; //ambient reflection
    float3 lightVec = normalize(lightPos.xyz - i.worldPos);
    float3 halfVec = normalize(viewVec + lightVec);
    color += lightColor * surfaceColor.xyz * kd * saturate(dot(normal, lightVec)); //diffuse reflection
    color += lightColor * ks * pow(saturate(dot(normal, halfVec)), m); //specular reflection
    return float4(saturate(color), 1.f);
}