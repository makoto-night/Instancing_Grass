#include "ModelRenderer.h"
#include "DirectX11Manager.h"

extern CommonConstantBuffer g_commonBuffer;
extern ConstantBuffer g_cBuff;

ModelRenderer::ModelRenderer()
{
	rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0), XMConvertToRadians(0), XMConvertToRadians(0));
	g_manager.CreateConstantBuffer(sizeof(XMFLOAT4), &MaterialBuffer);
	SetShader("Assets/Shaders/StandardShader.hlsl");
}

void ModelRenderer::SetRotation(float x, float y, float z)
{
	XMMATRIX tmpMtx = XMMatrixRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));
	rotationMatrix = tmpMtx;
}

void ModelRenderer::Rotate(float x, float y, float z)
{
	XMMATRIX tmpMtx = XMMatrixRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));
	rotationMatrix *= tmpMtx;
}

void ModelRenderer::LoadAssimp(const char* filename)
{
	Assimp::Importer importer;

	const aiScene* pScene = aiImportFile(filename,
		aiProcess_ConvertToLeftHanded |
		aiProcessPreset_TargetRealtime_MaxQuality);

	if (pScene == nullptr)
		return;

	// AI�m�[�h����͂���
	ProcessNode(pScene->mRootNode, pScene, filename);
}

void ModelRenderer::ProcessNode(aiNode* node, const aiScene* scene, string filename)
{
	// �m�[�h���̃��b�V���̐������[�v����
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		int meshindex = node->mMeshes[i];			// �m�[�h��i�Ԗڃ��b�V���̃C���f�b�N�X���擾
		aiMesh* mesh = scene->mMeshes[meshindex];	// �V�[�����烁�b�V���{�̂����o��

		ProcessMesh(mesh, scene, filename);
	}

	// �q�m�[�h�ɂ��Ă����
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, filename);
	}
}

void ModelRenderer::ProcessMesh(aiMesh* mesh, const aiScene* scene, string filename)
{
	ModelData tmpData;
	vector<VertexData> vertices;			// ���_
	vector<unsigned int> indices;		// �ʂ̍\�����
	vector<Material> textures;			// �e�N�X�`��

	// ���_�����擾
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		VertexData vertex;

		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;

		// �@���x�N�g�������݂��邩�H
		if (mesh->HasNormals()) {
			vertex.nor.x = mesh->mNormals[i].x;
			vertex.nor.y = mesh->mNormals[i].y;
			vertex.nor.z = mesh->mNormals[i].z;
		}
		else {
			vertex.nor.x = 0.0f;
			vertex.nor.y = 0.0f;
			vertex.nor.z = 0.0f;
		}

		// �e�N�X�`�����W�i�O�Ԗځj�����݂��邩�H
		if (mesh->HasTextureCoords(0)) {
			vertex.tex.x = mesh->mTextureCoords[0][i].x;
			vertex.tex.y = mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	// �e�N�X�`�������擾����
	if (mesh->mMaterialIndex >= 0)
	{
		// ���̃��b�V���̃}�e���A���C���f�b�N�X�l���擾����
		int	mtrlidx = mesh->mMaterialIndex;

		// �V�[������}�e���A���f�[�^���擾����
		aiMaterial* mat = scene->mMaterials[mtrlidx];

		// ���̃}�e���A���Ɋ֘A�Â����e�N�X�`�������o��
		tmpData.materialName = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse", scene, filename);
	}

	std::string a = tmpData.materialName;
	for (int i = 0; i < material.size(); i++)
	{
		if (strcmp(material[i].name, a.c_str()) == 0)
			tmpData.materialIndex = i;
	}

	// �ʂ̍\�������擾
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	tmpData.vertexData = vertices;
	tmpData.index = indices;

	modelData.push_back(tmpData);
}

string ModelRenderer::loadMaterialTextures(aiMaterial* mtrl, aiTextureType type, string typeName, const aiScene* scene, string filename)
{
	Material tmpMaterial;		// ���̃}�e���A���Ɋ֘A�Â���DIFFUSE�e�N�X�`���̃��X�g
	char textureFilename[256] = "";
	
	strcpy(tmpMaterial.name, mtrl->GetName().C_Str());

	// �}�e���A������e�N�X�`�������擾��(��{��1��)���[�v����
	for (unsigned int i = 0; i < mtrl->GetTextureCount(type); i++)
	{
		aiString str;

		// �}�e���A�����炉�Ԗڂ̃e�N�X�`���t�@�C�������擾����
		mtrl->GetTexture(type, i, &str);

		std::string texName = str.C_Str();
		char filepath[256] = { 0 };
		char tmp[256] = { 0 };
		int cnt = 0;
		for (int i = 0; i < 256; i++)
		{
			tmp[cnt] = filename[i];
			cnt++;
			if (filename[i] == '/')
			{
				tmp[i + 1] = '\0';
				strcat(filepath, tmp);
				ZeroMemory(tmp, sizeof(tmp));
				cnt = 0;
			}
			if (filename[i] == '\0')
				break;
		}
		strcpy(textureFilename, filepath);
		strcat(textureFilename, texName.c_str());

		bool skip = false;
		for (int i = 0; i < material.size(); i++)
		{
			if (strcmp(material[i].name, textureFilename) == 0)
			{
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		tmpMaterial.texture.Attach(g_manager.CreateTextureFromFile(textureFilename));
		strcpy(tmpMaterial.name, textureFilename);
		aiColor4D aiColor;
		aiGetMaterialColor(mtrl, "Diffuse", type, i, &aiColor);
		tmpMaterial.diffuse = { aiColor.r,aiColor.g,aiColor.b,aiColor.a };
		material.push_back(tmpMaterial);
	}

	if (mtrl->GetTextureCount(type) == 0)
	{

		aiColor4D aiColor;
		mtrl->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
		tmpMaterial.diffuse = { aiColor.r,aiColor.g,aiColor.b,aiColor.a };
	}

	if (mtrl->GetTextureCount(type) == 0)
	{
		tmpMaterial.texture.Attach(g_manager.CreateTextureFromFile("assets/white.png"));
		material.push_back(tmpMaterial);
	}

	return tmpMaterial.name;
}


void ModelRenderer::SetShader(string filename)
{
	shader = new ShaderElement;
	shader->SetShader(filename);
}

void ModelRenderer::SetOptionalTexture(string filename)
{
	optionalTexture.Attach(g_manager.CreateTextureFromFile(filename));
}

void ModelRenderer::UpdateBuffer()
{
	for (int i = 0; i < modelData.size(); i++)
	{
		modelData[i].VertexBuffer.Attach(g_manager.CreateVertexBuffer(modelData[i].vertexData.data(), modelData[i].vertexData.size()));
		modelData[i].IndexBuffer.Attach(g_manager.CreateIndexBuffer(modelData[i].index.data(), modelData[i].index.size()));
	}
}

void ModelRenderer::Draw()
{
	UpdateBuffer();

	g_manager.m_pImContext->IASetInputLayout(shader->InputLayout.Get());
	g_manager.m_pImContext->VSSetShader(shader->VertexShader.Get(), nullptr, 0);
	g_manager.m_pImContext->GSSetShader(shader->GeometryShader.Get(), nullptr, 0);
	g_manager.m_pImContext->PSSetShader(shader->PixelShader.Get(), nullptr, 0);

	g_commonBuffer.mtxWorld = XMMatrixTranspose(
		XMMatrixScaling(scale.x, scale.y, scale.z) *
		rotationMatrix *
		XMMatrixTranslationFromVector(position));

	g_manager.m_pImContext->UpdateSubresource(g_cBuff.Get(), 0, nullptr, &g_commonBuffer, 0, 0);
	g_manager.m_pImContext->VSSetConstantBuffers(0, 1, g_cBuff.GetAddressOf());
	g_manager.m_pImContext->GSSetConstantBuffers(0, 1, g_cBuff.GetAddressOf());
	g_manager.m_pImContext->PSSetConstantBuffers(0, 1, g_cBuff.GetAddressOf());


	for (int j = 0; j < modelData.size(); j++)
	{
		//Texture�̃Z�b�g
		if (optionalTexture != nullptr)
		{
			material[modelData[j].materialIndex].texture = optionalTexture;
		}
		else if (material[modelData[j].materialIndex].texture == NULL)
		{
			material[modelData[j].materialIndex].texture.Attach(g_manager.CreateTextureFromFile("Assets/Textures/white.png"));
		}
		g_manager.m_pImContext->PSSetShaderResources(0, 1, material[modelData[j].materialIndex].texture.GetAddressOf());


		//material���̍X�V
		g_manager.m_pImContext->UpdateSubresource(g_cBuff.Get(), 0, nullptr, &g_commonBuffer, 0, 0);

		//VertexBuffer��IndexBuffer���Z�b�g
		UINT hOffsets = 0, hStrides = sizeof(VertexData);
		g_manager.m_pImContext->IASetVertexBuffers(0, 1, modelData[j].VertexBuffer.GetAddressOf(), &hStrides, &hOffsets);
		g_manager.m_pImContext->IASetIndexBuffer(modelData[j].IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


		g_manager.m_pImContext->UpdateSubresource(MaterialBuffer.Get(), 0, nullptr, &material[modelData[j].materialIndex].diffuse, 0, 0);
		g_manager.m_pImContext->PSSetConstantBuffers(1, 1, MaterialBuffer.GetAddressOf());

		g_manager.m_pImContext->DrawIndexed((UINT)modelData[j].index.size(), 0, 0);
	}
}
