#include "Header.hlsl"

cbuffer cbTesselation: register(b0) {
	int4 tesselationSetting;
}

HSBicubicConstOutput CalcHSPatchConstants(
	InputPatch<PSIn, CONTROL_POINTS_PATCH> ip,
	uint PatchID : SV_PrimitiveID)
{
	HSBicubicConstOutput output;

    output.edges[0] = output.edges[2] = tesselationSetting.y;
    output.edges[1] = output.edges[3] = tesselationSetting.x;
    output.inside[0] = tesselationSetting.x;
    output.inside[1] = tesselationSetting.y;

    int2 segments = tesselationSetting.zw;
    int2 segment = int2(PatchID % segments.x, PatchID / segments.x);
    output.start = float2(segment.xy) / float2(segments.xy);
    output.end = output.start + 1.f / segments;

	return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(CONTROL_POINTS_PATCH)]
[patchconstantfunc("CalcHSPatchConstants")]
DSIn main(
	InputPatch<PSIn, CONTROL_POINTS_PATCH> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	DSIn output;

	output.pos = ip[i].pos;
	output.col = ip[i].col;

	return output;
}
