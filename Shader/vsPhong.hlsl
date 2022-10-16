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

PSPhongIn main(VSPhongIn i)
{
    PSPhongIn o;
    o.worldPos = mul(worldMatrix, float4(i.pos, 1.0f)).xyz;
    o.pos = mul(viewMatrix, float4(o.worldPos, 1.0f));
    o.pos = mul(projMatrix, o.pos);
    o.norm = mul(worldMatrix, float4(i.norm, 0.0f)).xyz;
    o.norm = normalize(o.norm);
    float3 camPos = mul(invViewMatrix, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    o.viewVec = camPos - o.worldPos;
    return o;
}