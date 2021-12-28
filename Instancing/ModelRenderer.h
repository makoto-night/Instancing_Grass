#pragma once

#include "DirectX11Manager.h"
#include	<assimp\Importer.hpp>
#include	<assimp\scene.h>
#include	<assimp\postprocess.h>
#include	<assimp/cimport.h>
#pragma comment (lib,"assimp-vc142-mt.lib")

extern DirectX11Manager g_manager;

struct CommonConstantBuffer
{
	XMMATRIX mtxWorld;
	XMMATRIX mtxView;
	XMMATRIX mtxProj;
	XMFLOAT4 lightDir{ 1,1,0,0 };
	XMFLOAT4 time = { 0,0,0,0 };
};

struct ShaderElement
{
	ComPtr<ID3D11VertexShader> VertexShader;
	ComPtr<ID3D11GeometryShader> GeometryShader;
	ComPtr<ID3D11PixelShader> PixelShader;
	ComPtr<ID3D11PixelShader> ShadowMap;
	ComPtr<ID3D11InputLayout> InputLayout;

	void SetShader(string filename)
	{
		VertexShader.Attach(g_manager.CreateVertexShader(filename, "vsMain"));
		GeometryShader.Attach(g_manager.CreateGeometryShader(filename, "gsMain"));
		PixelShader.Attach(g_manager.CreatePixelShader(filename, "psMain"));
		InputLayout.Attach(g_manager.CreateInputLayoutReflection(filename, "vsMain"));
	};
};

class ModelRenderer
{
public:
	//頂点情報
	struct VertexData
	{
		XMFLOAT3 pos; // 場所
		XMFLOAT4 intense = { 1,1,1,1 }; // 頂点カラー
		XMFLOAT2 tex = { 1,0 }; // テクスチャuv値
		XMFLOAT3 nor; // 法線

		bool operator ==(const VertexData data)
		{
			return memcmp(this, &data, sizeof(VertexData)) == 0;
		}
	};
	//マテリアル(テクスチャと質感のセット)
	struct Material
	{
		char name[256]; // マテリアルの名前
		XMFLOAT4 ambient; // 環境光
		XMFLOAT4 diffuse; // 拡散反射光
		XMFLOAT4 specular; // 鏡面反射
		float  shininess = 80; //鏡面反射の強さ
		ComPtr<ID3D11ShaderResourceView> texture; //テクスチャ情報
	};
	//マテリアル以外のデータ
	struct ModelData
	{
		string materialName;
		int materialIndex = 0; //何番目のマテリアルを使用するか
		//CPU側の頂点データ、添字データ
		vector<VertexData> vertexData;
		vector<UINT> index;
		//GPU側の頂点データ、添え字データ
		ComPtr<ID3D11Buffer> VertexBuffer;
		ComPtr<ID3D11Buffer> IndexBuffer;
	};

	vector<ModelData> modelData; //モデルのオブジェクトデータ配列

private:
	vector<Material> material; //マテリアル配列

	ConstantBuffer MaterialBuffer; //GPU側のマテリアル情報

	ShaderElement* shader;

	ComPtr<ID3D11ShaderResourceView> optionalTexture = nullptr; //テクスチャ情報
	XMFLOAT3 rotation = { 0,0,0 };

public:
	XMVECTOR position = XMVectorSet(0, 0, 0, 0);
	XMMATRIX rotationMatrix;
	XMFLOAT3 scale = { 1,1,1 };

	ModelRenderer();
	~ModelRenderer() {};

	void SetRotation(float x, float y, float z);
	void Rotate(float x, float y, float z);

	void LoadAssimp(const char* filename);
	void ProcessNode(aiNode* node, const aiScene* scene, string filename);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, string filename);
	string loadMaterialTextures(aiMaterial* mtrl, aiTextureType type, string typeName, const aiScene* scene, string filename);

	void SetShader(string filename);
	void SetOptionalTexture(string filename);

	void UpdateBuffer();

	void Draw();
};
