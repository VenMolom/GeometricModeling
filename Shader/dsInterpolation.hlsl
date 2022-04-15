#include "Header.hlsl"

cbuffer cbView : register(b0)
{
	matrix viewMatrix;
	matrix invViewMatrix;
};

cbuffer cbProj : register(b1)
{
	matrix projMatrix;
};

float4 horner(OutputPatch<VSOut, CONTROL_POINTS_INTERPOLATION> patch, float t)
{
    return float4(((patch[0].pos.xyz * t + patch[1].pos.xyz) * t + patch[2].pos.xyz) * t + patch[3].pos.xyz, 1.0f);
}

[domain("isoline")]
VSOut main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<VSOut, CONTROL_POINTS_INTERPOLATION> patch)
{
	VSOut output;

    float segmentLength = patch[3].col.x;
    float t = segmentLength * (1.0f/input.edges[0] * uv.x + uv.y);
    output.pos = mul(projMatrix, mul(viewMatrix, horner(patch, t)));
    //output.pos = horner(patch, t);
    output.col = patch[0].col;

	return output;
}