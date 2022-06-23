#include "Header.hlsl"

Texture2D trimTex : register(t0);
SamplerState samp : register(s0);

cbuffer cbColor: register(b0) {
	float4 color;
}

cbuffer cbTrim: register(b3) {
	float4 trim;
}

float4 main(PSIn i) : SV_TARGET
{
    float tex = trimTex.Sample(samp, i.tex).r;
    if (abs(tex - trim.x) > 0.5f) {
        discard;
    }
    return float4(color.rgb * color.a + i.col.rgb * (1 - color.a), 1.0f);
}