#pragma once

#include "DirectXLib.h"
#include "UtilityHeaders.h"

struct FBXVertex
{
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 tangent = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT2 texCoord = { 0.0f, 0.0f };
};

struct FBXMeshData
{
	std::vector<FBXVertex> vertices;
	std::vector<UINT> indices;
	std::string name;
	UINT materialIndex = 0;
};

struct FBXMaterialData
{
	std::string name;
	DirectX::XMFLOAT3 diffuseColor = { 0.8f, 0.8f, 0.8f };
	DirectX::XMFLOAT3 specularColor = { 0.2f, 0.2f, 0.2f };
	float shininess = 32.0f;
	std::string diffuseTexture;
	std::string normalTexture;
	std::string specularTexture;
};

struct FBXModelData
{
	std::vector<FBXMeshData> meshes;
	std::vector<FBXMaterialData> materials;
	std::string modelPath;
	bool hasAnimations = false;
	UINT totalVertices = 0;
	UINT totalTriangles = 0;
};

class FBXFileParser
{
public:
	static std::unique_ptr<FBXModelData> LoadFBXModel(const std::string& filePath);
	static void PrintModelInfo(const FBXModelData& modelData);

private:
	static FBXVertex ProcessVertex(const aiMesh* mesh, UINT vertexIndex);
	static FBXMaterialData ProcessMaterial(const aiMaterial* material, const std::string& modelDirectory);
	static FBXMeshData ProcessMesh(const aiMesh* mesh, const aiScene* scene);
	static void ProcessNode(const aiNode* node, const aiScene* scene, FBXModelData& modelData);
	static DirectX::XMFLOAT3 ConvertToXMFLOAT3(const aiVector3D& vec);
	static DirectX::XMFLOAT2 ConvertToXMFLOAT2(const aiVector3D& vec);
	static std::string ExtractDirectory(const std::string& filePath);
};