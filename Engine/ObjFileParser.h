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
	struct VertexKey
	{
		int positionIndex;
		int normalIndex;
		int uvIndex;

		bool operator==(const VertexKey& other) const
		{
			return positionIndex == other.positionIndex &&
				normalIndex == other.normalIndex &&
				uvIndex == other.uvIndex;
		}
	};

	struct VertexKeyHash
	{
		std::size_t operator()(const VertexKey& key) const
		{
			return std::hash<int>()(key.positionIndex) ^
				(std::hash<int>()(key.normalIndex) << 1) ^
				(std::hash<int>()(key.uvIndex) << 2);
		}
	};

	static void CalculateTangents(std::vector<Vertex>& vertices, const std::vector<UINT>& indices);

public:
	ObjFileParser(const std::wstring& filename);

	std::vector<DirectX::XMFLOAT3> m_positions;
	std::vector<DirectX::XMFLOAT3> m_normals;
	std::vector<DirectX::XMFLOAT2> m_uvs;

	struct Shape
	{
		std::wstring name;
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
	};

	std::vector<Shape> m_shapes;
};