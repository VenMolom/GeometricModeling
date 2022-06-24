#include "Header.hlsl"

static const float4x4 deBoorToBezier = float4x4(
        1.f / 6.f, 4.f / 6.f, 1.f / 6.f, 0,
		0, 2.f / 3.f, 1.f / 3.f, 0,
		0, 1.f / 3.f, 2.f / 3.f, 0,
		0, 1.f / 6.f, 4.f / 6.f, 1.f / 6.f
);

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

float4 deBoorPatch(OutputPatch<DSIn, CONTROL_POINTS_PATCH> patch, float2 uv)
{
    float4 b[4];
    float4x4 controls;
    for (int i = 0; i < 4; ++i)
    {
        controls = float4x4(patch[4 * i].pos, patch[4 * i + 1].pos, patch[4 * i + 2].pos, patch[4 * i + 3].pos);
        controls = mul(deBoorToBezier, controls);
        float4 p[4] = { controls._11_12_13_14, controls._21_22_23_24, controls._31_32_33_34, controls._41_42_43_44 };
        b[i] = deCastillo(p, uv.y);
    }

    controls = float4x4(b[0], b[1], b[2], b[3]);
    controls = mul(deBoorToBezier, controls);
    float4 bb[4] = { controls._11_12_13_14, controls._21_22_23_24, controls._31_32_33_34, controls._41_42_43_44 };

    return deCastillo(bb, uv.x);
}

[domain("quad")]
PSIn main(
	HSBicubicConstOutput input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<DSIn, CONTROL_POINTS_PATCH> patch)
{
    PSIn output;

    output.pos = deBoorPatch(patch, uv);
    output.col = patch[0].col;
    output.tex = float2(lerp(input.start.x, input.end.x, uv.x), lerp(input.start.y, input.end.y, uv.y));

    return output;
}