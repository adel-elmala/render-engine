
struct PointLight
{
	float3 position;
	// float padding_0;
	float3 color;
	float padding_1;
	float intensity;
	// float3 padding_2;
};

struct Material
{
	float3 ka;
	float3 kd;
	float3 ks;
	float padding;
	uint ns;
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

cbuffer mtl: register(b2)
{
	Material mtl;
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
	float3 color : COLOR;
	float2 uv :TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
	float4 pos_ws = mul(modelWorld, input.pos);
	float4 pos_cs = mul(worldCamera, pos_ws);
	float4 normal_ws = mul(modelWorld, input.normal);
	float4 normal_cs = mul(worldCamera, normal_ws);

	// ambient component
	float3 ambient_color = light.intensity * mtl.ka;

	// diffuse component
	float3 light_dir = normalize( mul(worldCamera,float4(light.position,1)).xyz - pos_cs.xyz);
	float3 incident_light = light.intensity * clamp(dot(normal_cs.xyz, light_dir), 0, 1);
	float3 diffuse_color = incident_light * mtl.kd;

	// specular component
	float3 view_dir = -normalize(pos_cs.xyz);
	float3 half_dir = (view_dir + light_dir) / length(view_dir + light_dir);
	float3 specular_color = incident_light * pow(clamp(dot(normal_cs.xyz, half_dir), 0, 1), 9000) * mtl.ks;

	VS_Output output;
	output.pos = mul(cameraNDC, pos_cs);
	output.color = ambient_color + diffuse_color + specular_color ;
	output.uv = input.uv;
	return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
	return clamp(t.Sample(s, input.uv) * float4(input.color,1.0), 0.0,1.0);
	// return float4(input.color,1.0);
	// return t.Sample(s, input.uv) ;
}