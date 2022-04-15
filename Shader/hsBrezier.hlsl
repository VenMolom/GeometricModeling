#include "Header.hlsl"

cbuffer cbTesselation: register(b0) {
	int4 tesselationSetting;
}

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VSOut, CONTROL_POINTS_BREZIER> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;

    output.edges[0] = tesselationSetting.x;
    output.edges[1] = 64.0f;

	return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(CONTROL_POINTS_BREZIER)]
[patchconstantfunc("CalcHSPatchConstants")]
VSOut main(
	InputPatch<VSOut, CONTROL_POINTS_BREZIER> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	VSOut output;

	output.pos = ip[i].pos;
	output.col = ip[i].col;

    if (PatchID == tesselationSetting.y && i >= tesselationSetting.z) {
        output.pos.w = 0.0f;
    }

	return output;
}
