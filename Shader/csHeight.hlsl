Texture2D<float> heightMap : register(t0);
RWTexture2D<float> outputTexture : register(u0);

cbuffer cbHeightParams : register(b0)
{
    int4 size; // x - width; y - height; z - tool size
    float4 material;
}

float readInputConverted(uint2 loc)
{
    float height;
    if (loc.x < 0 || loc.x >= size.x || loc.y < 0 || loc.y >= size.y) {
        height = 1.f;
    } else {
        height = heightMap[loc];
    }
    return (1.f - height) * material.z;
}

float toolHeight(float2 delta)
{
    float R = 1.f / size.z;
    float d = dot(delta, delta);
    return 0.5f * R * d;
}

static const float safeOffset = 2.f;

[numthreads(16, 16, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchID : SV_DispatchThreadID)
{
    float2 mmPerPixel = float2(material.x / size.x, material.y / size.y);
    uint2 pixelsConcerned = uint2(size.z * size.x / material.x, size.z * size.y / material.y);

    uint2 pixel = uint2(dispatchID.x, dispatchID.y);

    float baseHeight = readInputConverted(pixel);
    float heightMod = 0.f;

    int2 startEnd = pixelsConcerned / 2;
    for (int i = -startEnd.x; i < startEnd.x; ++i) {
        for (int j = -startEnd.y; j < startEnd.y; ++j) {
            int2 delta = int2(i, j);
            float currentHeight = baseHeight + toolHeight(float2(delta) * mmPerPixel);
            float pixelHeight = readInputConverted(pixel + delta);
            heightMod = max(heightMod, pixelHeight - currentHeight);
        }
    }

    outputTexture[pixel] = baseHeight + heightMod + safeOffset;
}