#include "Header.hlsl"

cbuffer cbColor: register(b0) {
	float4 color;
}

cbuffer cbFar : register(b1) {
    float4 nearFar;
}

float4 main(PSIn i) : SV_TARGET
{
    float dist = saturate((i.pos.w) / (nearFar.y - nearFar.x));
    return float4(color.rgb * dist + i.col.rgb * (1 - dist), 1.0f);
}