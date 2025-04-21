
cbuffer mats : register(b0)
{
	float4x4 modelWorld;
	float4x4 worldCamera;
	float4x4 cameraNDC;
};

struct VS_Input {
	float4 pos : POS;
};

float4 vs_main(VS_Input input) : SV_Position  
{
	float4 pos_ws = mul(modelWorld, input.pos);
	float4 pos_cs = mul(worldCamera, pos_ws);
	float4 pos_ndc = mul(cameraNDC, pos_cs);
	pos_ndc /= pos_ndc.w;
    return pos_ndc;
}

float4 ps_main() : SV_Target
{
	return float4(1, 0, 0, 1);
}