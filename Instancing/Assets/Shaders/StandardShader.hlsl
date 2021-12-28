Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix mtxWorld;
    matrix mtxView;
    matrix mtxProj;
    float4 lightDir;
    float4 time;
}

cbuffer ConstantMaterial : register(b1)
{
    float4 diffuse;
}

struct VS_INPUT
{
    float3 pos : Position;
    float4 col : COLOR;
    float2 tex : TEXCOORD;
    float3 nor : NORMAL;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float4 worldPos : WRL_Position;
    float4 col : COLOR;
    float2 tex : TEXCOORD;
    float3 nor : NORMAL;
};

PS_INPUT vsMain(VS_INPUT input)
{
    PS_INPUT output;

    float4 localPosition = float4(input.pos, 1.0f);
    float4 worldPosition = mul(localPosition, mtxWorld);
    float4 viewPosition = mul(worldPosition, mtxView);
    float4 projPosition = mul(viewPosition, mtxProj);

    output.pos = projPosition;
    output.worldPos = worldPosition;
    output.tex = input.tex;
    output.col = input.col;
    output.nor = normalize(mul(input.nor, (float3x3)mtxWorld));

    return output;
}

float4 psMain(PS_INPUT input) : SV_Target
{
    float4 tex = txDiffuse.Sample(samLinear, input.tex);
    float col;
    col = dot(input.nor, normalize(lightDir.xyz));
    col = saturate(col);
    col = col * 0.5f + 0.5f;

    float3 color = tex.rgb * diffuse * col;

    return float4(color, tex.a * input.col.a);
}