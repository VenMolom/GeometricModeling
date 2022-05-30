#include "Header.hlsl"

float4 deCastillo(float4 b[4], float t)
{
    float t1 = 1.0f - t;

    for (int j = 3; j > 0; --j)
    {
        for (int i = 0; i < j; ++i)
        {
            b[i] = b[i] * t1 + b[i + 1] * t;
        }
    }

    return b[0];
}

float4 deCastilloPatch(OutputPatch<DSIn, CONTROL_POINTS_PATCH> patch, float2 uv)
{
    float4 b[4];
    
    for (int i = 0; i < 4; ++i)
    {
        float4 p[4] = { patch[4 * i].pos, patch[4 * i + 1].pos, patch[4 * i + 2].pos, patch[4 * i + 3].pos };
        b[i] = deCastillo(p, uv.y);
    }

    return deCastillo(b, uv.x);
}

[domain("quad")]
PSIn main(
	HSBicubicConstOutput input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<DSIn, CONTROL_POINTS_PATCH> patch)
{
	PSIn output;

    output.pos = deCastilloPatch(patch, uv);
    output.col = patch[0].col;

	return output;
}