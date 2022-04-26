#include "Header.hlsl"

cbuffer cbFar : register(b0) {
    float4 nearFar;
}

cbuffer cbProj : register(b1)
{
	matrix projMatrix;
};

static const float size = 1.2f;

[maxvertexcount(4)]
void main(point PSIn inArray[1], inout TriangleStream<PSIn> ostream)
{
    PSIn i = inArray[0];

    if (i.pos.z > 0) return;

    float dist = saturate(abs(i.pos.z) / abs(nearFar.y - nearFar.x));
	float diff = size * dist;

	PSIn o = (PSIn)0;
	o.col = i.col;

	o.pos = mul(projMatrix, i.pos + float4(-diff, 0, 0, 0));
	ostream.Append(o);

	o.pos = mul(projMatrix, i.pos + float4(0, diff, 0, 0));
    ostream.Append(o);

    o.pos = mul(projMatrix, i.pos + float4(0, -diff, 0, 0));
    ostream.Append(o);

    o.pos = mul(projMatrix, i.pos + float4(diff, 0, 0, 0));
    ostream.Append(o);

	ostream.RestartStrip();
}