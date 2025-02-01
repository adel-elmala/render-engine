
cbuffer constants : register(b0)
{
     float4x4 modelViewProj;
};

struct VS_Input {
    float4 pos : POS;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, (modelViewProj));
    // output.pos = mul(  transpose(modelViewProj), input.pos);
    output.color = input.pos + float3(0.5f, 0.5f, 0.5f);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return float4(1.0,0.0,0.0, 1.0); 
}