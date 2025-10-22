#pragma once

#include "UtilityHeaders.h"
#include "DirectXLib.h"

#include "Component.h"

extern std::unordered_map<std::wstring, UINT> g_meshIdMap;
extern std::unordered_map<std::wstring, UINT> g_vertexShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_geometryShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_pixelShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_textureIdMap;

struct ShapeData
{
	UINT meshId = 0;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT vertexShaderId = 0;
	UINT pixelShaderId = 0;

	std::vector<UINT> textureIds = {};
};

extern std::vector<std::pair<Object*, ShapeData*>> g_renderShapes;

class Shape : public Component
{
	ShapeData m_renderData = {};

public:
	Shape
	(
		const std::wstring& mesh,
		const std::wstring& vertexShader = L"VertexShader",
		const std::wstring& pixelShader = L"PixelShader",
		const std::vector<std::wstring>& textures = { L"NoTexture" }
	);

	void OnAttached(class Object* owner) override { Component::OnAttached(owner); g_renderShapes.emplace_back(owner, &m_renderData); }
	void OnDetached() override;
};