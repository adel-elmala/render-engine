cbuffer mats : register(b0)
{
	float4x4 modelWorld;
	float4x4 worldCamera;
	float4x4 cameraNDC;
};

struct VS_Input {
	float4 pos : POS;
	float4 normal : NORMAL;
	float2 uv : TEX;
};

struct VS_Output {
	float4 pos : SV_POSITION;
	float depth : TEXCOORD0;
};

VS_Output vs_main(VS_Input input)
{
	float4 pos_ws = mul(modelWorld, input.pos);
	float4 pos_cs = mul(worldCamera, pos_ws);
	float4 pos_ndc = mul(cameraNDC, pos_cs);

	VS_Output output;
	output.pos = pos_ndc;
	output.pos /= output.pos.w;
	output.depth = output.pos.z;
	return output;
}

float ps_main(VS_Output input) : SV_Depth
{
	return input.depth;
}