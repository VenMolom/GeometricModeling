#include "Header.hlsl"

cbuffer cbPoints: register(b0) {
	float4 _points;
}


float4 deCastillio(OutputPatch<VSOut, NUM_CONTROL_POINTS> patch, float t)
{
    float4 b[4] = {patch[0].pos, patch[1].pos, patch[2].pos, patch[3].pos};
    float t1 = 1.0f - t;

    for (int j = _points.x - 1; j > 0; --j) {
        for (int i = 0; i < j; ++i) {
            b[i] = b[i] * t1 + b[i + 1] * t;
        }
    }

    return b[0];
}

[domain("isoline")]
VSOut main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<VSOut, NUM_CONTROL_POINTS> patch)
{
	VSOut output;

    float t = 1/64.0f * uv.x + uv.y;
    output.pos = deCastillio(patch, t);
    output.col = patch[0].col;

	return output;
}