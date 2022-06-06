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

float4 deCastilloPatch(float4 patch[16], float2 uv)
{
    float4 b[4];

    for (int i = 0; i < 4; ++i)
    {
        float4 p[4] = { patch[4 * i], patch[4 * i + 1], patch[4 * i + 2], patch[4 * i + 3] };
        b[i] = deCastillo(p, uv.x);
    }

    return deCastillo(b, uv.y);
}

void convertPatch(in OutputPatch<DSIn, CONTROL_POINTS_GREGORY> patch, in float2 uv, out float4 convertedPatch[16])
{
    convertedPatch[0] = patch[0].pos;
    convertedPatch[1] = patch[1].pos;
    convertedPatch[2] = patch[2].pos;
    convertedPatch[3] = patch[3].pos;

    convertedPatch[4] = patch[4].pos;
    convertedPatch[7] = patch[9].pos;
    convertedPatch[8] = patch[10].pos;
    convertedPatch[11] = patch[15].pos;

    convertedPatch[12] = patch[16].pos;
    convertedPatch[13] = patch[17].pos;
    convertedPatch[14] = patch[18].pos;
    convertedPatch[15] = patch[19].pos;

    convertedPatch[5] = (uv.x * patch[6].pos + uv.y * patch[5].pos) / (uv.x + uv.y + 0.00001f);
    convertedPatch[6] = ((1 - uv.x) * patch[7].pos + uv.y * patch[8].pos) / (1 - uv.x + uv.y + 0.00001f);
//     convertedPatch[5] = (uv.y * patch[6].pos + uv.x * patch[5].pos) / (uv.x + uv.y + 0.00001f);
//     convertedPatch[6] = ((1 - uv.y) * patch[7].pos + uv.x * patch[8].pos) / (1 - uv.y + uv.x + 0.00001f);
    //convertedPatch[5] = lerp(convertedPatch[4], convertedPatch[7], 0);
    //convertedPatch[6] = lerp(convertedPatch[4], convertedPatch[7], 1);

    convertedPatch[9] = (uv.x * patch[12].pos + (1 - uv.y) * patch[11].pos) / (1 + uv.x - uv.y + 0.00001f);
    convertedPatch[10] = ((1 - uv.x) * patch[13].pos + (1 - uv.y) * patch[14].pos) / (2 - uv.x - uv.y + 0.00001f);
//     convertedPatch[9] = (uv.y * patch[12].pos + (1 - uv.x) * patch[11].pos) / (1 + uv.y - uv.x + 0.00001f);
//     convertedPatch[10] = ((1 - uv.y) * patch[13].pos + (1 - uv.x) * patch[14].pos) / (2 - uv.x - uv.y + 0.00001f);
    //convertedPatch[9] = lerp(convertedPatch[8], convertedPatch[11], 0);
    //convertedPatch[10] = lerp(convertedPatch[8], convertedPatch[11], 1);
}

[domain("quad")]
PSIn main(
	HSBicubicConstOutput input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<DSIn, CONTROL_POINTS_GREGORY> patch)
{
	PSIn output;

    float4 convertedPatch[16];
    convertPatch(patch, uv, convertedPatch);

    output.pos = deCastilloPatch(convertedPatch, uv);
    output.col = patch[0].col;

	return output;
}