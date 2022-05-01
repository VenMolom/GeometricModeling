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
    return float4(leftColor * left + rightColor * right, 1.f);
}