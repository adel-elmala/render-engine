
cbuffer constants : register(b0)
{
    float4x4 modelWorld;
    float4x4 worldCamera;
    float4x4 cameraNDC;
};

Texture2D t: register(t0);
SamplerState s: register(s0);



struct VS_Input {
    float4 pos : POS;
    float2 uv : TEX;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    // float3 color : COLOR;
    float2 uv :TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    // float4 inter = mul(modelWorld,input.pos);
    // float4 inter2 = mul(worldCamera,inter);
    // output.pos = mul(cameraNDC, inter2);
    output.pos = mul(cameraNDC,mul(worldCamera,mul(modelWorld, input.pos)));
    // output.color = input.pos + float3(0.5f, 0.5f, 0.5f);
    output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return t.Sample(s, input.uv);
    // return float4(input.color,1.0bunny/bunny.obj); 
}