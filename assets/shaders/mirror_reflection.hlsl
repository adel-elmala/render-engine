cbuffer mats : register(b0)
{
	float4x4 modelWorld;
	float4x4 worldCamera;
	float4x4 cameraNDC;
};

Texture2D t: register(t0);
SamplerState s: register(s0);

struct VS_Input {
	float4 pos : POS;
	float4 normal : NORMAL;
	float2 uv : TEX;
};

struct VS_Output {
	float4 pos : SV_POSITION;
};

VS_Output vs_main(VS_Input input)
{
	float4 pos_ws = mul(modelWorld, input.pos);
	float4 pos_cs = mul(worldCamera, pos_ws);
	float4 pos_ndc = mul(cameraNDC, pos_cs);

	VS_Output output;
	output.pos = pos_ndc;
	output.pos /= output.pos.w;
	return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
	uint rtv_width , rtv_height, levels;
	t.GetDimensions(0, rtv_width, rtv_height, levels);
	float2 uv = float2( input.pos.x / rtv_width,  input.pos.y / rtv_height); 
	return t.Sample(s, uv);
}