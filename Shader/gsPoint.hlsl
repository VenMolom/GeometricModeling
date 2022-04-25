#include "Header.hlsl"

cbuffer cbFar : register(b0) {
    float4 farPlane;
}

static const float size = .012f;

[maxvertexcount(4)]
void main(point PSIn inArray[1], inout TriangleStream<PSIn> ostream)
{
    PSIn i = inArray[0];
    float dist = saturate(i.pos.z / i.pos.w);
	float diff = size * dist * i.pos.w;

	PSIn o = (PSIn)0;
	o.col = i.col;

	o.pos = i.pos + float4(-diff, 0, 0, 0);
	ostream.Append(o);

	o.pos = i.pos + float4(0, diff, 0, 0);
    ostream.Append(o);

    o.pos = i.pos + float4(0, -diff, 0, 0);
    ostream.Append(o);

    o.pos = i.pos + float4(diff, 0, 0, 0);
    ostream.Append(o);

	ostream.RestartStrip();
}