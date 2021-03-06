#include <chrono>
#include <thread>


#include "DirectX11Manager.h"
#include "Input.h"
#include "ModelRenderer.h"
#include "PoissonGenerator.h"

vector<XMVECTOR> CreateGrassPositionData(ModelRenderer::ModelData& modelData, float diameter, float density);
VertexBuffer CreateGrassMatrixBuffer(vector<XMVECTOR> grassPosData);

DirectX11Manager g_manager;
CommonConstantBuffer g_commonBuffer;
ConstantBuffer g_cBuff;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	if (FAILED(g_manager.Init(hInstance, nCmdShow)))
		return -1;


	//Shaderを作成
	VertexShader vs;
	PixelShader ps;
	InputLayout inputLayout;
	vs.Attach(g_manager.CreateVertexShader("Assets/Shaders/GrassShader.hlsl", "vsMain"));
	ps.Attach(g_manager.CreatePixelShader("Assets/Shaders/GrassShader.hlsl", "psMain"));

	D3D11_INPUT_ELEMENT_DESC a;
	//InputLayoutの作成
	D3D11_INPUT_ELEMENT_DESC elem[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{
			"INSTANCE_MTX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_INSTANCE_DATA, 1
		},
		{
			"INSTANCE_MTX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_INSTANCE_DATA, 1
		},
		{
			"INSTANCE_MTX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_INSTANCE_DATA, 1
		},
		{
			"INSTANCE_MTX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_INSTANCE_DATA, 1
		},
	};
	inputLayout.Attach(g_manager.CreateInputLayout(elem, 8, "Assets/Shaders/GrassShader.hlsl", "vsMain"));

	//頂点情報を設定
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT4 col;
		XMFLOAT2 uv;
		XMFLOAT3 nor;
	};
	//地面に対して垂直な横長長方形ポリゴンを作成
	vector<Vertex> vertexs =
	{
		{XMFLOAT3(-0.5f, -0.1f, 0.1f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(0, 1), XMFLOAT3(1,0,0)},
		{XMFLOAT3(0.5f, -0.1f, 0.1f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(1, 1), XMFLOAT3(1,0,0)},
		{XMFLOAT3(0.5f, 1.0f, 0.1f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(1, 0), XMFLOAT3(1,0,0)},
		{XMFLOAT3(-0.5f, 1.0f, 0.1f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(0, 0), XMFLOAT3(1,0,0)}
	};

	//長方形ポリゴンの座標だけ別に取り出す(計算用)
	vector<XMVECTOR> polyPositions =
	{
		{vertexs[0].pos.x, vertexs[0].pos.y, vertexs[0].pos.z},
		{vertexs[1].pos.x, vertexs[1].pos.y, vertexs[1].pos.z},
		{vertexs[2].pos.x, vertexs[2].pos.y, vertexs[2].pos.z},
		{vertexs[3].pos.x, vertexs[3].pos.y, vertexs[3].pos.z}
	};

	//最初に作成した1枚の長方形ポリゴンを元に、60°ずつ回転させることで3枚の長方形が組み合わさった草用のメッシュを作成する
	for (int i = 0; i < 2; i++)
	{
		//回転用クォータニオンを作成
		XMVECTOR rotQuaternion = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(0),
			XMConvertToRadians(60),
			XMConvertToRadians(0));

		for (int i = 0; i < 4; i++) //頂点4つ分
		{
			//頂点座標と法線を回転
			XMVECTOR position = polyPositions[i];
			position = XMVector3Rotate(position, rotQuaternion);
			XMVECTOR normal = XMVectorSet(vertexs[i].nor.x, vertexs[i].nor.y, vertexs[i].nor.z, 1);
			normal = XMVector3Rotate(normal, rotQuaternion);

			//頂点リストに追加
			vertexs.push_back({
				XMFLOAT3(position.m128_f32[0], position.m128_f32[1], position.m128_f32[2]),
				vertexs[i].col,
				vertexs[i].uv,
				XMFLOAT3(normal.m128_f32[0],normal.m128_f32[1],normal.m128_f32[2])
				});
		}
	}

	//バーテックスバッファの作成
	VertexBuffer vb;
	vb.Attach(g_manager.CreateVertexBuffer(vertexs.data(), static_cast<UINT>(vertexs.size())));

	//インデックス情報の設定
	vector<UINT> idxs = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11 };
	IndexBuffer ib;
	ib.Attach(g_manager.CreateIndexBuffer(idxs.data(), static_cast<UINT>(idxs.size())));

	//テクスチャの作成
	ShaderTexture texture;
	texture.Attach(g_manager.CreateTextureFromFile("Assets/Textures/Grass.png"));

	//カメラ情報
	float yAngle = 0, xAngle = 30;
	XMVECTOR cameraPos = { 0,20,-40,0 };
	XMMATRIX cameraMtx;

	//コンスタントバッファ
	g_manager.CreateConstantBuffer(sizeof(CommonConstantBuffer), &g_cBuff);
	g_commonBuffer.mtxProj = XMMatrixTranspose(
		XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 1270.0f / 760.0f, 0.5f, 16384.0f));
	g_commonBuffer.mtxWorld = XMMatrixIdentity();
	g_commonBuffer.lightDir = { 1,1,0,0 };

	//草を生やすベースになる島のモデルを読み込み
	ModelRenderer islandRenderer;
	islandRenderer.LoadAssimp("Assets/Models/Island.fbx");

	//草を生やすポイントのリストを作成
	float grassDensity = 200;
	auto grassPosData = CreateGrassPositionData(islandRenderer.modelData[1], 60, grassDensity);

	//インスタンシング用のバッファを作成
	VertexBuffer instancingMtxBuff = CreateGrassMatrixBuffer(grassPosData);


	MSG msg = { nullptr };
	while (true)
	{
		const auto deltaTime = static_cast<float>(timeGetTime());
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (WM_QUIT == msg.message) return 0;

		//インプットの更新とカメラ操作
		Input::Update();

		if (Input::GetMouseButton(MOUSE_MIDDLE_BUTTON))
		{
			cameraPos += cameraMtx.r[1] * Input::GetMouseMovePos().Y * 0.01f;
			cameraPos -= cameraMtx.r[0] * Input::GetMouseMovePos().X * 0.01f;
		}
		if (Input::GetMouseButton(MOUSE_LEFT_BUTTON))
		{
			xAngle += Input::GetMouseMovePos().Y * 0.1f;
			yAngle += Input::GetMouseMovePos().X * 0.1f;
		}
		cameraPos += cameraMtx.r[2] * Input::GetMoveMouseWheel() * 0.01f;

		cameraMtx = XMMatrixIdentity() * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(XMConvertToRadians(xAngle), XMConvertToRadians(yAngle), 0));

		if (Input::GetKeyDown(DIK_UP))
		{
			grassDensity += 10;
			grassPosData = CreateGrassPositionData(islandRenderer.modelData[1], 60, grassDensity);
			instancingMtxBuff = CreateGrassMatrixBuffer(grassPosData);
		}
		if (Input::GetKeyDown(DIK_DOWN))
		{
			grassDensity -= 10;
			grassDensity = std::max(0.0f, grassDensity);
			grassPosData = CreateGrassPositionData(islandRenderer.modelData[1], 60, grassDensity);
			instancingMtxBuff = CreateGrassMatrixBuffer(grassPosData);
		}
		if (Input::GetKeyDown(DIK_RIGHT))
		{
			grassDensity += 100;
			grassPosData = CreateGrassPositionData(islandRenderer.modelData[1], 60, grassDensity);
			instancingMtxBuff = CreateGrassMatrixBuffer(grassPosData);
		}
		if (Input::GetKeyDown(DIK_LEFT))
		{
			grassDensity -= 100;
			grassDensity = std::max(0.0f, grassDensity);
			grassPosData = CreateGrassPositionData(islandRenderer.modelData[1], 60, grassDensity);
			instancingMtxBuff = CreateGrassMatrixBuffer(grassPosData);
		}

		//カメラ更新
		g_commonBuffer.mtxView = XMMatrixTranspose(XMMatrixLookAtLH(
			cameraPos, cameraPos + cameraMtx.r[2], cameraMtx.r[1]));
		//シェーダ―で使うための時間を加算(1フレームにつき1.0f加算)
		g_commonBuffer.time.x++;

		g_manager.m_pImContext->UpdateSubresource(g_cBuff.Get(), 0, nullptr, &g_commonBuffer, 0, 0);
		g_manager.m_pImContext->VSSetConstantBuffers(0, 1, g_cBuff.GetAddressOf());
		g_manager.m_pImContext->PSSetConstantBuffers(0, 1, g_cBuff.GetAddressOf());


		g_manager.DrawBegin();

		//ポリゴンを書くための各種パラメータセット
		g_manager.SetVertexShader(vs.Get());
		g_manager.SetPixelShader(ps.Get());
		g_manager.SetInputLayout(inputLayout.Get());


		//頂点情報としてセットで渡せるように配列にする
		ID3D11Buffer* const vbuffer[] = {
			vb.Get(),
			instancingMtxBuff.Get()
		};
		// それぞれのストライドをセット
		unsigned int stride[2] = { sizeof(Vertex), sizeof(XMMATRIX) };
		// オフセットをセット
		unsigned offset[2] = { 0, 0 };
		g_manager.m_pImContext->IASetVertexBuffers(0, 2, vbuffer, stride, offset);

		g_manager.SetIndexBuffer(ib.Get());
		g_manager.SetTexture2D(0, texture.Get());

		//DrawCall
		g_manager.m_pImContext->DrawIndexedInstanced(
			idxs.size(), // 描画するインデックス数(Face*3)
			static_cast<UINT>(grassPosData.size()), // 繰り返し描画回数
			0, // 最初のインデックスバッファの位置
			0, // 頂点バッファの最初から使う
			0);

		//島描画
		islandRenderer.Draw();

		g_manager.DrawEnd();

		//FPS制御
		const float nowTime = static_cast<float>(timeGetTime());
		const float fps = nowTime - deltaTime;
		const auto stop = static_cast<std::uint64_t>(60 - fps);
		if (fps < 1000.0f / 60)
			std::this_thread::sleep_for(std::chrono::microseconds(stop));
	}

	return 0;
}

struct Triangle
{
	XMVECTOR v0;
	XMVECTOR v1;
	XMVECTOR v2;
};

struct Hit
{
	bool hit = false;
	XMVECTOR pos;
	float distance = 0;
};


float Dot(XMVECTOR vecA, XMVECTOR vecB, XMVECTOR vecC)
{
	return ((vecA.m128_f32[0] * vecB.m128_f32[1] * vecC.m128_f32[2]) + (vecA.m128_f32[1] * vecB.m128_f32[2] * vecC.
		m128_f32[0]) + (vecA.m128_f32[2] * vecB.m128_f32[0] * vecC.m128_f32[1]) - (vecA.m128_f32[0] * vecB.m128_f32[2] *
			vecC.m128_f32[1]) - (vecA.m128_f32[1] * vecB.m128_f32[0] * vecC.m128_f32[2]) - (vecA.m128_f32[2] * vecB.m128_f32
				[1] * vecC.m128_f32[0]));
}

//レイと三角形ポリゴンとの当たり判定
Hit RayIntersectsTriangle(XMVECTOR origin, XMVECTOR ray, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2)
{
	Hit ret;

	XMVECTOR invRay = -ray;
	XMVECTOR edge1 = v1 - v0;
	XMVECTOR edge2 = v2 - v0;

	float denominator = Dot(edge1, edge2, invRay);

	if (denominator <= 0)
	{
		return ret;
	}

	XMVECTOR d = origin - v0;

	float u = Dot(d, edge2, invRay) / denominator;
	if ((u >= 0) && (u <= 1))
	{
		float v = Dot(edge1, d, invRay) / denominator;
		if ((v >= 0) && (u + v <= 1))
		{
			float t = Dot(edge1, edge2, d) / denominator;

			if (t < 0)
			{
				return ret;
			}

			XMVECTOR tmp = ray * t;
			ret.pos = origin + tmp;
			ret.hit = true;
			ret.distance = t;
		}
	}

	return ret;
}

//diameter=>直径,density=>密度(1*1の範囲あたり何本生やすか)
vector<XMVECTOR> CreateGrassPositionData(ModelRenderer::ModelData& modelData, float diameter, float density)
{
	//島のメッシュから三角形のリストを作成
	vector<Triangle> meshTriangles;
	vector<UINT> index = modelData.index;
	vector<ModelRenderer::VertexData> vertexData = modelData.vertexData;
	UINT i = 0;
	while (i < modelData.index.size())
	{
		Triangle tmpTriangle;
		tmpTriangle.v0 = XMVectorSet(vertexData[index[i]].pos.x, vertexData[index[i]].pos.y, vertexData[index[i]].pos.z, 1);
		i++;
		tmpTriangle.v1 = XMVectorSet(vertexData[index[i]].pos.x, vertexData[index[i]].pos.y, vertexData[index[i]].pos.z, 1);
		i++;
		tmpTriangle.v2 = XMVectorSet(vertexData[index[i]].pos.x, vertexData[index[i]].pos.y, vertexData[index[i]].pos.z, 1);
		i++;
		meshTriangles.push_back(tmpTriangle);
	}

	//草を生やすポイントを計算し作成する
	PoissonGenerator::DefaultPRNG PRNG;
	auto poissonPoints = PoissonGenerator::generatePoissonPoints(density * diameter, PRNG);
	vector<XMVECTOR> grassPositions;
	for (int i = 0; i < poissonPoints.size(); i++)
	{
		float grassPosX = (poissonPoints[i].x - 0.5f) * diameter;
		float grassPosY = (poissonPoints[i].y - 0.5f) * diameter;

		//レイを上から下に飛ばすため、yを高めに設定しておく
		XMVECTOR grassPos = XMVectorSet(grassPosX, 500, grassPosY, 0);

		//１つ１つのポリゴンに対して衝突判定を行う
		for (auto t : meshTriangles)
		{
			Hit hit = RayIntersectsTriangle(grassPos, XMVectorSet(0, -1000, 0, 0), t.v0, t.v1, t.v2);
			if (hit.hit) //Hitした場合草を生やすポジションとして追加する
				grassPositions.push_back(XMVectorSet(hit.pos.m128_f32[0], hit.pos.m128_f32[1], hit.pos.m128_f32[2], 1));
		}
	}
	return grassPositions;
}

VertexBuffer CreateGrassMatrixBuffer(vector<XMVECTOR> grassPosData)
{
	vector<XMMATRIX> instancingMtx;
	for (int i = 0; i < grassPosData.size(); i++)
	{
		XMVECTOR pos = grassPosData[i];
		XMMATRIX mtx = XMMatrixTranspose(XMMatrixTranslationFromVector(pos));
		instancingMtx.push_back(mtx);
	}
	VertexBuffer instancingMtxBuff;
	instancingMtxBuff.
		Attach(g_manager.CreateVertexBuffer(instancingMtx.data(), static_cast<int>(instancingMtx.size())));

	return instancingMtxBuff;
}