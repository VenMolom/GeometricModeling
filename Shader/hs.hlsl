#include "Header.hlsl"

cbuffer cbTesselation: register(b0) {
	float tesselationAmount;
}

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VSOut, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;

    output.edges[0] = 64.0f; // TODO: calculate correct values
    output.edges[1] = 64.0f;

	return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("CalcHSPatchConstants")]
VSOut main(
	InputPatch<VSOut, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	VSOut output;

	output.pos = ip[i].pos;
	output.col = ip[i].col;

	return output;
}
