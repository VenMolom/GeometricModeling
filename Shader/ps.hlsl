#include "Header.hlsl"

cbuffer cbColor: register(b0) {
	float4 color;
}

float4 main(VSOut i) : SV_TARGET
{
    if (color.w != 0.0f) {
        return color;
    } else {
	    return i.col;
    }
}