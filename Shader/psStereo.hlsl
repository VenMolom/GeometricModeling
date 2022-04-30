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
    float2 texCoords = i.pos.xy * .5f + float2(.5f, .5f);
    texCoords.y *= -1.f;

    float4 left = leftEye.Sample(colorSampler, texCoords);
    float4 right = rightEye.Sample(colorSampler, texCoords);
    return left;
}