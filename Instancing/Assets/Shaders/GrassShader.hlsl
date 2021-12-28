SamplerState samLinear : register(s0);
Texture2D Diffuse : register(t0);

cbuffer ConstantBuffer : register(b0)
{
	matrix mtxWorld;
	matrix mtxView;
	matrix mtxProj;
	float4 lightDir;
	float4 time;
}

struct VS_INPUT
{
	float3 Pos : POSITION;
	float4 Col : COLOR;
	float2 Tex : TEXCOORD;
	float3 Nor : NORMAL;
	matrix posMtx : INSTANCE_MTX;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Col : COL;
	float2 Tex : TEXCOORD;
	float3 Nor : NORMAL;
};

PS_INPUT vsMain(VS_INPUT input)
{
	PS_INPUT o = (PS_INPUT)0;

	float4 localPosition = float4(input.Pos, 1.0f);
	float4 worldPosition = mul(localPosition, input.posMtx);
	float4 viewPosition = mul(worldPosition, mtxView);
	float4 projPosition = mul(viewPosition, mtxProj);

	o.Pos = projPosition;
	o.Col = input.Col;
	o.Tex = input.Tex;
	o.Nor = mul(input.Nor, input.posMtx);
	return o;
}

float4 psMain(PS_INPUT input) : SV_TARGET
{
	float4 result = 0;
	result = Diffuse.Sample(samLinear, input.Tex) * input.Col;
	
	//�@���ƃ��C�g����Œ���̉A�e������
	float3 nor = normalize(input.Nor);
	float dLight = dot(nor, normalize(lightDir.xyz)) * 0.5 + 0.5;

	//�A���t�@�l���ق�0�ɂȂ镔���͕`�悵�Ȃ�(if(result.a - 0.9f < 0){discard;}�Ɠ�������)
	clip(result.a - 0.9f);

	return float4(result.rgb * dLight,1.0f);
}