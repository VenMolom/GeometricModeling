#include "Header.hlsl"

VSOut main( VSIn i )
{
	VSOut o;
	o.pos = float4(i.pos, 1.0f);
	o.col = float4(i.col, 1.0f);
	return o;
}