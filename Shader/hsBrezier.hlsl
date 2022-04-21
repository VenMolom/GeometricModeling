#include "Header.hlsl"

cbuffer cbTesselation: register(b0) {
	int4 tesselationSetting;
}

HSBrezierConstOutput CalcHSPatchConstants(
	InputPatch<PSIn, CONTROL_POINTS_BREZIER> ip,
	uint PatchID : SV_PrimitiveID)
{
	HSBrezierConstOutput output;

	float4 maxx = max(ip[0].pos / ip[0].pos.w, max(ip[1].pos, max(ip[2].pos, ip[3].pos)));
	float4 minn = min(ip[0].pos, min(ip[1].pos, min(ip[2].pos, ip[3].pos)));

    float2 start = clamp(minn.xy, float2(-1, -1), float2(1, 1));
    float2 end = clamp(maxx.xy, float2(-1, -1), float2(1, 1));

    float2 scale = float2(tesselationSetting.x * 0.5f, tesselationSetting.y * 0.5f);

    float2 startViewport = mad(scale, start, scale);
    float2 endViewport = mad(scale, end, scale);

    //float tesselationAmount = clamp(ceil(distance(startViewport, endViewport) / 64.0f), 1.0f, 64.0f);
    float tesselationAmount = clamp(ceil(distance(startViewport, endViewport)), 1.0f, 64.0f);

    //output.edges[0] = tesselationAmount;
    output.edges[0] = 1.0f;
    output.edges[1] = tesselationAmount;
    output.patchSize = PatchID == tesselationSetting.z ? tesselationSetting.w : 4;

	return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(CONTROL_POINTS_BREZIER)]
[patchconstantfunc("CalcHSPatchConstants")]
DSIn main(
	InputPatch<PSIn, CONTROL_POINTS_BREZIER> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	DSIn output;

	output.pos = ip[i].pos;
	output.col = ip[i].col;

	return output;
}
