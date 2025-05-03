
cbuffer mat : register(b0)
{
	float4x4 NDCWorld;
};

TextureCube env: register(t0);
SamplerState s: register(s0);

struct VS_Input {
	uint vID : SV_VertexID;
};

struct VS_Output {
	float4 pos : SV_POSITION;
	float3 uv :TEXCOORD0;
};

VS_Output vs_main(VS_Input input)
{
	float4 rect_positions[6];
	rect_positions[0] = float4( -1, -1, 0.99999, 1);
	rect_positions[1] = float4( 1, -1, 0.99999, 1);
	rect_positions[2] = float4( -1, 1, 0.99999, 1);
	rect_positions[3] = float4( 1, -1, 0.99999, 1);
	rect_positions[4] = float4( 1, 1, 0.99999, 1);
	rect_positions[5] = float4( -1, 1, 0.99999, 1);

	float4 rect_position_world = mul(NDCWorld, rect_positions[input.vID]);
	float3 rect_uv = rect_position_world.xyz;

	VS_Output output;
	output.pos = rect_positions[input.vID];
	output.uv = rect_uv;
	return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
	float4 texel = env.Sample(s, input.uv);
	return texel;
}