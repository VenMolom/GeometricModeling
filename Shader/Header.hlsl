struct VSIn {
	float3 pos : POSITION;
	float3 col : COLOR;
};

struct VSOut {
	float4 pos : SV_POSITION;
	float4 col: COLOR;
};

struct HS_CONSTANT_DATA_OUTPUT {
	float edges[2] : SV_TessFactor;
};

#define NUM_CONTROL_POINTS 4