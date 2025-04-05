
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
	float3 uv :TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
	float4 rect_positions[6];
	rect_positions[0] = float4( -1, -1, 0.999, 1);
	rect_positions[1] = float4( 1, -1, 0.999, 1);
	rect_positions[2] = float4( -1, 1, 0.999, 1);
	rect_positions[3] = float4( 1, -1, 0.999, 1);
	rect_positions[4] = float4( 1, 1, 0.999, 1);
	rect_positions[5] = float4( -1, 1, 0.999, 1);

	float3 rect_uvs[6];
	rect_uvs[0] = normalize(mul(NDCWorld, rect_positions[0]).xyz);
	rect_uvs[1] = normalize(mul(NDCWorld, rect_positions[1]).xyz);
	rect_uvs[2] = normalize(mul(NDCWorld, rect_positions[2]).xyz);
	rect_uvs[3] = normalize(mul(NDCWorld, rect_positions[3]).xyz);
	rect_uvs[4] = normalize(mul(NDCWorld, rect_positions[4]).xyz);
	rect_uvs[5] = normalize(mul(NDCWorld, rect_positions[5]).xyz);

	VS_Output output;
	output.pos = rect_positions[input.vID];
	output.uv = rect_uvs[input.vID];
	return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
	float4 texel = env.Sample(s, input.uv);
	return texel;
}