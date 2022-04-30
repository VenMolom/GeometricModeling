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
    float4 left = leftEye.Sample(colorSampler, i.tex);
    float4 right = rightEye.Sample(colorSampler, i.tex);
    return .5f * left + .5f * right;
}