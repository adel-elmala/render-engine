
struct PointLight
{
	float3 position;
	// float padding_0;
	float3 color;
	float padding_1;
	float intensity;
	// float3 padding_2;
};



cbuffer mats : register(b0)
{
	float4x4 modelWorld;
	float4x4 worldCamera;
	float4x4 cameraNDC;
};

cbuffer light: register(b1)
{ 
	PointLight light;
};

struct VS_Input {
	float4 pos : POS;
	float4 normal : NORMAL;
	float2 uv : TEX;
};

struct VS_Output {
	float4 pos : SV_POSITION;
	float4 color : TEXCOORD0;
	float2 uv :TEXCOORD1;
};

VS_Output vs_main(VS_Input input)
{
	float3 ka = float3(0.2,0.2,0.2);
	float3 kd = float3(0.2,0.2,0.2);
	float3 ks = float3(0.2,0.2,0.2);


	float4 pos_ws = mul(modelWorld, input.pos);
	float4 pos_cs = mul(worldCamera, pos_ws);
	float4 normal_ws = mul(modelWorld, input.normal);
	float4 normal_cs = mul(worldCamera, normal_ws);
	normal_cs = normalize(normal_cs);

	// ambient component
	float3 ambient_color = ka;

	// diffuse component
	float3 light_dir = normalize(mul(worldCamera, float4(light.position, 1)).xyz - pos_cs.xyz);
	float NdotL = clamp(dot(normal_cs.xyz, light_dir), 0, 1);
	float3 incident_light = float3(1.0,1.0,1.0) * light.intensity * NdotL;
	float3 diffuse_color = incident_light * kd;

	// specular component
	float3 view_dir = -normalize(pos_cs.xyz);
	float3 half_dir = normalize(view_dir + light_dir);
	float3 specular_color = incident_light * pow(clamp(dot(normal_cs.xyz, half_dir), 0, 1), 300) * ks;

	VS_Output output;
	output.pos = mul(cameraNDC, pos_cs);
	output.pos /= output.pos.w;
	output.color.xyz = diffuse_color + specular_color;
	output.color.w = 1.0;
	output.uv = input.uv;
	return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
	return input.color;
	// return t.Sample(s, input.uv) ;
}