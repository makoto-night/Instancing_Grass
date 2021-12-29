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

//疑似乱数
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

	//座標が0に近い場所は草の背丈を低くする
	localPosition.y *= input.posMtx[3].y;
	localPosition.y = clamp(0, 1.0, localPosition.y);

	float4 worldPosition = mul(localPosition, input.posMtx);

	float trailTime = time.x * 0.02f;//揺らす速度
	float trailMagnitude = 0.1f;//揺らす幅

	//現在の時間(フレーム)とsin関数を使って疑似的に風に揺れているようにする
	float3 windAdd = float3(sin(trailTime + worldPosition.x * 0.1f) * trailMagnitude, 0, 0);

	//揺れにばらつきを持たせるためのランダム数値
	float random = GetRandomNumber(float2(worldPosition.x, worldPosition.y), 0) * 100;
	//全部一緒に揺れると不自然なので適度にばらつかせる
	float3 positionAdd = float3(sin(trailTime - random) * trailMagnitude, 0, 0);

	//葉先だけ揺らすためにローカル座標のyを掛ける
	positionAdd *= input.Pos.y;
	windAdd *= input.Pos.y;
	//ワールド座標に揺れを加算する
	worldPosition.xyz += positionAdd + windAdd;

	float4 viewPosition = mul(worldPosition, mtxView);
	float4 projPosition = mul(viewPosition, mtxProj);

	o.Pos = projPosition;
	o.Col = input.Col;
	o.Tex = input.Tex;

	//メッシュの法線ではなく、揺れを元に法線の代わりにする
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

	//法線とライトから最低限の陰影をつける
	float3 nor = normalize(input.Nor);
	float dLight = dot(nor, normalize(lightDir.xyz)) * 0.5 + 0.5;

	//アルファ値がほぼ0になる部分は描画しない(if(result.a - 0.9f < 0){discard;}と同じ効果)
	clip(result.a - 0.9f);

	return float4(result.rgb * dLight,1.0f);
}