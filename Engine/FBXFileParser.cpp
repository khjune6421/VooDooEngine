#include "FBXFileParser.h"
#include <iostream>
#include <filesystem>

using namespace std;
using namespace DirectX;

std::unique_ptr<FBXModelData> FBXFileParser::LoadFBXModel(const std::string& filePath)
{
	Assimp::Importer importer;

	// Configure import settings for FBX
	//importer.SetPropertyInteger(AI_CONFIG_IMPORT_FBX_READ_ALL_GEOMETRY_LAYERS, 1);
	//importer.SetPropertyInteger(AI_CONFIG_IMPORT_FBX_READ_ALL_MATERIALS, 1);
	//importer.SetPropertyInteger(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, 1);

	// Load the model with comprehensive post-processing
	const aiScene* scene = importer.ReadFile(filePath,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_LimitBoneWeights |
		aiProcess_SplitLargeMeshes |
		aiProcess_ValidateDataStructure |
		aiProcess_FindDegenerates |
		aiProcess_FindInvalidData |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_OptimizeMeshes
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR: Failed to load FBX model: " << filePath << "\n";
		cout << "Assimp Error: " << importer.GetErrorString() << "\n";
		return nullptr;
	}

	auto modelData = make_unique<FBXModelData>();
	modelData->modelPath = filePath;
	modelData->hasAnimations = scene->mNumAnimations > 0;

	// Process materials first
	string modelDirectory = ExtractDirectory(filePath);
	modelData->materials.reserve(scene->mNumMaterials);
	for (UINT i = 0; i < scene->mNumMaterials; i++)
	{
		modelData->materials.push_back(ProcessMaterial(scene->mMaterials[i], modelDirectory));
	}

	// Process the scene hierarchy
	ProcessNode(scene->mRootNode, scene, *modelData);

	// Calculate totals
	for (const auto& mesh : modelData->meshes)
	{
		modelData->totalVertices += static_cast<UINT>(mesh.vertices.size());
		modelData->totalTriangles += static_cast<UINT>(mesh.indices.size()) / 3;
	}

	cout << "Successfully loaded FBX model: " << filesystem::path(filePath).filename().string() << "\n";
	PrintModelInfo(*modelData);

	return modelData;
}

void FBXFileParser::ProcessNode(const aiNode* node, const aiScene* scene, FBXModelData& modelData)
{
	// Process all meshes in the current node
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		modelData.meshes.push_back(ProcessMesh(mesh, scene));
	}

	// Recursively process child nodes
	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, modelData);
	}
}

FBXMeshData FBXFileParser::ProcessMesh(const aiMesh* mesh, const aiScene* scene)
{
	FBXMeshData meshData;
	meshData.name = mesh->mName.length > 0 ? mesh->mName.C_Str() : "UnnamedMesh";
	meshData.materialIndex = mesh->mMaterialIndex;

	// Process vertices
	meshData.vertices.reserve(mesh->mNumVertices);
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		meshData.vertices.push_back(ProcessVertex(mesh, i));
	}

	// Process indices
	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			meshData.indices.push_back(face.mIndices[j]);
		}
	}

	return meshData;
}

FBXVertex FBXFileParser::ProcessVertex(const aiMesh* mesh, UINT vertexIndex)
{
	FBXVertex vertex;

	// Position
	if (mesh->mVertices)
	{
		vertex.position = ConvertToXMFLOAT3(mesh->mVertices[vertexIndex]);
	}

	// Normal
	if (mesh->mNormals)
	{
		vertex.normal = ConvertToXMFLOAT3(mesh->mNormals[vertexIndex]);
	}

	// Tangent
	if (mesh->mTangents)
	{
		vertex.tangent = ConvertToXMFLOAT3(mesh->mTangents[vertexIndex]);
	}

	// Texture coordinates (use first set if available)
	if (mesh->mTextureCoords[0])
	{
		vertex.texCoord = ConvertToXMFLOAT2(mesh->mTextureCoords[0][vertexIndex]);
	}

	// Vertex colors (use first set if available)
	if (mesh->mColors[0])
	{
		const aiColor4D& color = mesh->mColors[0][vertexIndex];
		vertex.color = XMFLOAT4(color.r, color.g, color.b, color.a);
	}

	return vertex;
}

FBXMaterialData FBXFileParser::ProcessMaterial(const aiMaterial* material, const std::string& modelDirectory)
{
	FBXMaterialData matData;

	// Material name
	aiString name;
	if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
	{
		matData.name = name.C_Str();
	}

	// Diffuse color
	aiColor3D diffuse(0.8f, 0.8f, 0.8f);
	material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	matData.diffuseColor = XMFLOAT3(diffuse.r, diffuse.g, diffuse.b);

	// Specular color
	aiColor3D specular(0.2f, 0.2f, 0.2f);
	material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
	matData.specularColor = XMFLOAT3(specular.r, specular.g, specular.b);

	// Shininess
	float shininess = 32.0f;
	material->Get(AI_MATKEY_SHININESS, shininess);
	matData.shininess = shininess;

	// Textures
	aiString texturePath;
	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
	{
		matData.diffuseTexture = modelDirectory + "/" + texturePath.C_Str();
	}

	if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
	{
		matData.normalTexture = modelDirectory + "/" + texturePath.C_Str();
	}

	if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS)
	{
		matData.specularTexture = modelDirectory + "/" + texturePath.C_Str();
	}

	return matData;
}

DirectX::XMFLOAT3 FBXFileParser::ConvertToXMFLOAT3(const aiVector3D& vec)
{
	return XMFLOAT3(vec.x, vec.y, vec.z);
}

DirectX::XMFLOAT2 FBXFileParser::ConvertToXMFLOAT2(const aiVector3D& vec)
{
	return XMFLOAT2(vec.x, vec.y);
}

std::string FBXFileParser::ExtractDirectory(const std::string& filePath)
{
	size_t lastSlash = filePath.find_last_of("/\\");
	return (lastSlash != string::npos) ? filePath.substr(0, lastSlash) : "";
}

void FBXFileParser::PrintModelInfo(const FBXModelData& modelData)
{
	cout << "  Meshes: " << modelData.meshes.size() << "\n";
	cout << "  Materials: " << modelData.materials.size() << "\n";
	cout << "  Total Vertices: " << modelData.totalVertices << "\n";
	cout << "  Total Triangles: " << modelData.totalTriangles << "\n";
	cout << "  Has Animations: " << (modelData.hasAnimations ? "Yes" : "No") << "\n";

	for (size_t i = 0; i < modelData.meshes.size(); ++i)
	{
		const auto& mesh = modelData.meshes[i];
		cout << "    Mesh " << i << ": " << mesh.name
			<< " (Vertices: " << mesh.vertices.size()
			<< ", Triangles: " << mesh.indices.size() / 3 << ")\n";
	}
}