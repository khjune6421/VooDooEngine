#pragma once
#include "pch.h"

struct Vertex
{
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 tangent = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT2 uv = { 0.0f, 0.0f };
};

class ObjFileParser
{
	static void CalculateTangents(std::vector<Vertex>& vertices);

public:
	ObjFileParser(const std::wstring& filename);

	struct Mesh
	{
		std::wstring name;
		std::vector<Vertex> vertices;
	};
	std::vector<DirectX::XMFLOAT3> m_positions;
	std::vector<DirectX::XMFLOAT3> m_normals;
	std::vector<DirectX::XMFLOAT2> m_uvs;

	std::vector<Mesh> m_meshes;
};