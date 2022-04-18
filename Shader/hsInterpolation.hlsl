#include "Header.hlsl"

cbuffer cbTesselation: register(b0) {
	int4 tesselationSetting;
}

cbuffer cbView : register(b1) {
	matrix viewMatrix;
	matrix invViewMatrix;
};

cbuffer cbProj : register(b2) {
	matrix projMatrix;
};

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VSOut, CONTROL_POINTS_INTERPOLATION> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;

	float2 start = clamp(mul(projMatrix, mul(viewMatrix, ip[3].pos)).xy, float2(-1, -1), float2(1, 1));
    float2 end = clamp(mul(projMatrix, mul(viewMatrix, ip[3].col)).xy, float2(-1, -1), float2(1, 1));

    float2 scale = float2(tesselationSetting.x * 0.5f, tesselationSetting.y * 0.5f);

    float2 startViewport = mad(scale, start, scale);
    float2 endViewport = mad(scale, end, scale);

    float tesselationAmount = clamp(ceil(distance(startViewport, endViewport) / 64.0f), 1.0f, 64.0f);

    output.edges[0] = tesselationAmount;
    output.edges[1] = 64.0f;

	return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(CONTROL_POINTS_INTERPOLATION)]
[patchconstantfunc("CalcHSPatchConstants")]
VSOut main(
	InputPatch<VSOut, CONTROL_POINTS_INTERPOLATION> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	VSOut output;

	output.pos = ip[i].pos;
	output.col = ip[i].col;

	return output;
}
