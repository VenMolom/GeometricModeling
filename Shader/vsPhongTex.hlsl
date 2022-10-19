#include "Header.hlsl"
Texture2D heightMap : register(t0);
SamplerState heightSampler : register(s0);

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

PSPhongIn main(VSPhongTexIn i)
{
    PSPhongIn o;
    i.pos.z *= heightMap.SampleLevel(heightSampler, i.tex, 0).x;
    o.worldPos = mul(worldMatrix, float4(i.pos, 1.0f)).xyz;
    o.pos = mul(viewMatrix, float4(o.worldPos, 1.0f));
    o.pos = mul(projMatrix, o.pos);
    o.norm = mul(worldMatrix, float4(i.norm, 0.0f)).xyz;
    o.norm = normalize(o.norm);
    float3 camPos = mul(invViewMatrix, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    o.viewVec = camPos - o.worldPos;
    o.tex = i.tex;
    return o;
}