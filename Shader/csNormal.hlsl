Texture2D<float> heightMap : register(t0);
Texture2D<float> prevHeightMap : register(t1);
RWTexture2D<float4> outputTexture : register(u0);
RWTexture1D<uint> errorMap : register(u1);

cbuffer cbHeightParams : register(b0)
{
    int4 size; // x - width; y - height; z - tool type
}

float readInput(uint2 loc)
{
    if (loc.x < 0 || loc.x >= size.x || loc.y < 0 || loc.y >= size.y)
        return 0;
    return heightMap[loc];
}

float readInputPrev(uint2 loc)
{
    return prevHeightMap[loc];
}

float3 normal(float3 binormal, float3 tangent)
{
    return normalize(cross(normalize(binormal), normalize(tangent)));
}

[numthreads(16, 16, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchID : SV_DispatchThreadID)
{
    float2 h = 1.f / float2(size.xy);

    uint2 pixel = uint2(dispatchID.x, dispatchID.y);

    float left = readInput(pixel + uint2(-1, 0));
    float right = readInput(pixel + uint2(1, 0));
    float top = readInput(pixel + uint2(0, -1));
    float bottom = readInput(pixel + uint2(0, 1));

    float3 xDiff = float3(2 * h.x, 0, right - left);
    float3 yDiff = float3(0, 2 * h.y, top - bottom);

    float3 norm = normal(xDiff, yDiff);

    norm = mad(norm, 0.5f, 0.5f);

    outputTexture[pixel] = float4(norm, 1.f);

    float current = readInput(pixel);
    float prev = readInputPrev(pixel);
    float diff = current - prev;

    // TODO: check for errors
    InterlockedAdd(errorMap[0], 0);
}