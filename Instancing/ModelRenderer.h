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
	//���_���
	struct VertexData
	{
		XMFLOAT3 pos; // �ꏊ
		XMFLOAT4 intense = { 1,1,1,1 }; // ���_�J���[
		XMFLOAT2 tex = { 1,0 }; // �e�N�X�`��uv�l
		XMFLOAT3 nor; // �@��

		bool operator ==(const VertexData data)
		{
			return memcmp(this, &data, sizeof(VertexData)) == 0;
		}
	};
	//�}�e���A��(�e�N�X�`���Ǝ����̃Z�b�g)
	struct Material
	{
		char name[256]; // �}�e���A���̖��O
		XMFLOAT4 ambient; // ����
		XMFLOAT4 diffuse; // �g�U���ˌ�
		XMFLOAT4 specular; // ���ʔ���
		float  shininess = 80; //���ʔ��˂̋���
		ComPtr<ID3D11ShaderResourceView> texture; //�e�N�X�`�����
	};
	//�}�e���A���ȊO�̃f�[�^
	struct ModelData
	{
		string materialName;
		int materialIndex = 0; //���Ԗڂ̃}�e���A�����g�p���邩
		//CPU���̒��_�f�[�^�A�Y���f�[�^
		vector<VertexData> vertexData;
		vector<UINT> index;
		//GPU���̒��_�f�[�^�A�Y�����f�[�^
		ComPtr<ID3D11Buffer> VertexBuffer;
		ComPtr<ID3D11Buffer> IndexBuffer;
	};

	vector<ModelData> modelData; //���f���̃I�u�W�F�N�g�f�[�^�z��

private:
	vector<Material> material; //�}�e���A���z��

	ConstantBuffer MaterialBuffer; //GPU���̃}�e���A�����

	ShaderElement* shader;

	ComPtr<ID3D11ShaderResourceView> optionalTexture = nullptr; //�e�N�X�`�����
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
