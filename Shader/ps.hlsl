#include "Header.hlsl"

cbuffer cbColor: register(b0) {
	float4 color;
}

float4 main(VSOut i) : SV_TARGET
{
    return float4(color.rgb * color.a + i.col.rgb * (1 - color.a), 1.0f);
}