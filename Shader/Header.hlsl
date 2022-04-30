struct VSIn {
	float3 pos : POSITION;
	float3 col : COLOR;
};

struct PSIn {
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

struct DSIn {
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

struct PSStereoIn {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

struct VSStereoIn {
    float2 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct HSBrezierConstOutput {
	float edges[2] : SV_TessFactor;
	int patchSize : PATCHSIZE;
};

#define CONTROL_POINTS_BREZIER 4