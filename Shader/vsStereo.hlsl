#include "Header.hlsl"

PSStereoIn main(VSStereoIn i)
{
	PSStereoIn o;
	o.pos = float4(i.pos, 0.0f, 1.0f);
	return o;
}