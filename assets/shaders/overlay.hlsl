
Texture2D renderTexture: register(t0);
SamplerState s: register(s0);

struct VS_Input {
	uint vID : SV_VertexID;
};

struct VS_Output {
	float4 pos : SV_POSITION;
	float2 uv :TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
	float2 rect_positions[6];
	rect_positions[0] = float2( -1, -1 );
	rect_positions[1] = float2( 1, -1 );
	rect_positions[2] = float2( -1, 1 );
	rect_positions[3] = float2( 1, -1 );
	rect_positions[4] = float2( 1, 1 );
	rect_positions[5] = float2( -1, 1 );
	
	float2 rect_uvs[6];
	rect_uvs[0] = float2( 0, 1 );
	rect_uvs[1] = float2( 1, 1 );
	rect_uvs[2] = float2( 0, 0 );
	rect_uvs[3] = float2( 1, 1 );
	rect_uvs[4] = float2( 1, 0 );
	rect_uvs[5] = float2( 0, 0 );
	
	VS_Output output;
	output.pos = float4(rect_positions[input.vID], 0, 1);
	output.uv = rect_uvs[input.vID];
	return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
	return renderTexture.Sample(s, input.uv);
}