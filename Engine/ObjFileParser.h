#pragma once

#include "DirectXLib.h"
#include "UtilityHeaders.h"

struct Vertex
{
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 color = { 1.0f, 0.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT2 uv = { 0.0f, 0.0f };
	DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
};
extern D3D11_INPUT_ELEMENT_DESC g_layoutDesc[4];

class ObjFileParser
{
public:
	ObjFileParser(const std::wstring& filename);

	struct Shape
	{
		std::wstring name;
		std::vector<Vertex> vertices;
	};

	std::vector<Shape> m_shapes;
};