cbuffer mats : register(b0)
{
	float4x4 modelWorld;
	float4x4 worldCamera;
	float4x4 cameraNDC;
};

cbuffer light_mats : register(b1)
{
	float4x4 _;
	float4x4 worldLight;
	float4x4 lightNDC;
};

Texture2D reflected_scene: register(t0);
Texture2D<float> light_depth: register(t1);
SamplerState s: register(s0);

struct VS_Input {
	float4 pos : POS;
	float4 normal : NORMAL;
	float2 uv : TEX;
};

struct VS_Output {
	float4 pos : SV_POSITION;
	float3 pos_in_light_view : TEXCOORD0;
};

VS_Output vs_main(VS_Input input)
{
	float4 pos_ws = mul(modelWorld, input.pos);
	float4 pos_cs = mul(worldCamera, pos_ws);
	float4 pos_ndc = mul(cameraNDC, pos_cs);

	float4 pos_light_cs = mul(worldLight, pos_ws);
	float4 pos_light_ndc = mul(lightNDC, pos_light_cs);
	pos_light_ndc /= pos_light_ndc.w;

	VS_Output output;
	output.pos = pos_ndc;
	output.pos /= output.pos.w;
	output.pos_in_light_view = pos_light_ndc.xyz;
	return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
	// uint rtv_width , rtv_height, levels;
	// reflected_scene.GetDimensions(0, rtv_width, rtv_height, levels);
	// float2 uv = float2(input.pos.x / rtv_width,  input.pos.y / rtv_height); 
	// float4 reflection =  reflected_scene.Sample(s, uv);

	// float2 depth_uv = input.pos_in_light_view.xy * float2(0.5f,0.5f) + float2(0.5f,0.5f);
	// bool in_shadow = light_depth.Sample(s, depth_uv) <= input.pos_in_light_view.z;

	// if (in_shadow)
	// 	return float4(0,0,0,1);
	// else
	// 	return reflection;
	return float4(0.6, 0.4, 0.3, 1.0);
}