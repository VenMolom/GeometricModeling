#include "Header.hlsl"

cbuffer cbWorld : register(b0)
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1)
{
	matrix viewMatrix;
	matrix invViewMatrix;
};

cbuffer cbProj : register(b2)
{
	matrix projMatrix;
};

VSOut main( VSIn i )
{
	VSOut o;
	o.pos = float4(i.pos, 1.0f);
    o.pos = mul(invViewMatrix, o.pos);
    o.pos = float4((o.pos - mul(invViewMatrix, float4(0, 0, 0, 1))).xyz, 1);
    o.pos = mul(projMatrix, mul(viewMatrix, mul(worldMatrix, o.pos)));
    o.col = float4(i.col, 1.0f);
	return o;
}