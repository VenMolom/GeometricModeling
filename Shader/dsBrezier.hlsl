#include "Header.hlsl"

float4 deCastillo(OutputPatch<DSIn, CONTROL_POINTS_BREZIER> patch, float t, int size)
{
    float4 b[4] = {patch[0].pos, patch[1].pos, patch[2].pos, patch[3].pos};
    float t1 = 1.0f - t;

    for (int j = size - 1; j > 0; --j) {
        for (int i = 0; i < j; ++i) {
            b[i] = b[i] * t1 + b[i + 1] * t;
        }
    }

    return b[0];
}

[domain("isoline")]
PSIn main(
	HSBrezierConstOutput input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<DSIn, CONTROL_POINTS_BREZIER> patch)
{
	PSIn output;

    float t = 1.0f/input.edges[0] * uv.x + uv.y;
    output.pos = deCastillo(patch, t, input.patchSize);
    output.col = patch[0].col;

	return output;
}