#include "Header.hlsl"

cbuffer cbWorld : register(b0)
{
    matrix worldMatrix;
};

PSIn main( VSIn i )
{
	PSIn o;
	o.pos = mul(worldMatrix, float4(i.pos, 1.0f));
	o.col = float4(i.col, 1.0f);
	return o;
}