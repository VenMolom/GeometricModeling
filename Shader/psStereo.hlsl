#include "Header.hlsl"

Texture2D leftEye : register(t0);
Texture2D rightEye : register(t1);
SamplerState colorSampler : register(s0);

cbuffer cbStereoColor: register(b2) {
	float4 leftColor;
	float4 rightColor;
}

float4 main(PSStereoIn i) : SV_TARGET
{
    float3 left = leftEye.Sample(colorSampler, i.tex).rgb;
    float3 right = rightEye.Sample(colorSampler, i.tex).rgb;

    float2 r = normalize(float2(leftColor.r, rightColor.r));
    float2 g = normalize(float2(leftColor.g, rightColor.g));
    float2 b = normalize(float2(leftColor.b, rightColor.b));

    float3 leftFilter = float3(r.x, g.x, b.x);
    float3 rightFilter = float3(r.y, g.y, b.y);

    return float4(leftFilter * left + rightFilter * right, 1.f);
}