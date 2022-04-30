#include "Header.hlsl"

PSStereoIn main(VSStereoIn i)
{
	PSStereoIn o;
	o.pos = float4(i.pos, 0.5f, 1.0f);
	o.tex = i.tex;
	return o;
}