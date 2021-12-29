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

//�^������
float GetRandomNumber(float2 texCoord, int Seed)
{
	return frac(sin(dot(texCoord.xy, float2(12.9898, 78.233)) + Seed) * 43758.5453);
}

float3 rotate(float3 p, float angle, float3 axis) {
	float3 a = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float r = 1.0 - c;
	float3x3 m = float3x3(
		a.x * a.x * r + c,
		a.y * a.x * r + a.z * s,
		a.z * a.x * r - a.y * s,
		a.x * a.y * r - a.z * s,
		a.y * a.y * r + c,
		a.z * a.y * r + a.x * s,
		a.x * a.z * r + a.y * s,
		a.y * a.z * r - a.x * s,
		a.z * a.z * r + c
		);
	return mul(p, m);
}

PS_INPUT vsMain(VS_INPUT input)
{
	PS_INPUT o = (PS_INPUT)0;

	float4 localPosition = float4(input.Pos, 1.0f);

	//���W��0�ɋ߂��ꏊ�͑��̔w���Ⴍ����
	localPosition.y *= input.posMtx[3].y;
	localPosition.y = clamp(0, 1.0, localPosition.y);

	float4 worldPosition = mul(localPosition, input.posMtx);

	float trailTime = time.x * 0.02f;//�h�炷���x
	float trailMagnitude = 0.1f;//�h�炷��

	//���݂̎���(�t���[��)��sin�֐����g���ċ^���I�ɕ��ɗh��Ă���悤�ɂ���
	float3 windAdd = float3(sin(trailTime + worldPosition.x * 0.1f) * trailMagnitude, 0, 0);

	//�h��ɂ΂�����������邽�߂̃����_�����l
	float random = GetRandomNumber(float2(worldPosition.x, worldPosition.y), 0) * 100;
	//�S���ꏏ�ɗh���ƕs���R�Ȃ̂œK�x�ɂ΂������
	float3 positionAdd = float3(sin(trailTime - random) * trailMagnitude, 0, 0);

	//�t�悾���h�炷���߂Ƀ��[�J�����W��y���|����
	positionAdd *= input.Pos.y;
	windAdd *= input.Pos.y;
	//���[���h���W�ɗh������Z����
	worldPosition.xyz += positionAdd + windAdd;

	float4 viewPosition = mul(worldPosition, mtxView);
	float4 projPosition = mul(viewPosition, mtxProj);

	o.Pos = projPosition;
	o.Col = input.Col;
	o.Tex = input.Tex;

	//���b�V���̖@���ł͂Ȃ��A�h������ɖ@���̑���ɂ���
	o.Nor = float3(1,0,1);
	float rotatePower = 3.0f;
	o.Nor = rotate(o.Nor, (positionAdd + windAdd) * rotatePower, float3(1, 0, 0));
	o.Nor = normalize(o.Nor);

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