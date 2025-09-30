#pragma once

#include "DirectXLib.h"
#include "UtilityHeaders.h"

class ObjFileParser
{
public:
	ObjFileParser(const std::wstring& filename);

	struct Vertex
	{
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4 color = { 1.0f, 0.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT2 uv = { 0.0f, 0.0f };
	};
	struct Face
	{
		Vertex vertices[3];
		DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
	};
	struct Shape
	{
		std::wstring name;
		std::vector<Face> faces;
	};
};