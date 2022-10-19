struct VSIn {
	float3 pos : POSITION;
	float3 col : COLOR;
};

struct PSIn {
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD0;
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

struct VSParamIn {
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct HSBrezierConstOutput {
	float edges[2] : SV_TessFactor;
	int patchSize : PATCHSIZE;
};

struct HSBicubicConstOutput {
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
	float2 start : TEXSTART;
	float2 end : TEXEND;
};

struct VSPhongIn {
    float3 pos : POSITION;
    float3 norm : NORMAL0;
};

struct VSPhongTexIn {
    float3 pos : POSITION;
    float3 norm : NORMAL0;
    float2 tex: TEXCOORD0;
};


struct PSPhongIn {
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 norm : NORMAL0;
    float3 viewVec : VIEWVEC0;
    float2 tex: TEXCOORD0;
};

#define CONTROL_POINTS_BREZIER 4

#define CONTROL_POINTS_PATCH 16
#define CONTROL_POINTS_GREGORY 20