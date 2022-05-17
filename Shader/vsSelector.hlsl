cbuffer cbModel : register(b0)
{
	matrix modelMatrix;
};

float4 main( float2 pos : POSITION ) : SV_POSITION
{
    float2 ndcPos = mul(modelMatrix, float4(pos, 0.f, 1.f)).xy;
	return float4(ndcPos, 0.0001f, 1.0f);
}