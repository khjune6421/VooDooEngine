#pragma once

#include "UtilityHeaders.h"
#include "DirectXLib.h"

#include "IComponent.h"

extern std::unordered_map<std::wstring, UINT> g_meshIdMap;
extern std::unordered_map<std::wstring, UINT> g_vertexShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_pixelShaderIdMap;

struct ShapeData
{
	UINT meshId = 0;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT vertexShaderId = 0;
	UINT pixelShaderId = 0;
};

extern std::vector<std::pair<Object*, ShapeData*>> g_renderShapes;

class Shape : public IComponent
{
	ShapeData m_renderData = {};

public:
	Shape(const std::wstring& mesh, const std::wstring& vertexShader = L"VertexShader", const std::wstring& pixelShader = L"PixelShader");
	~Shape() { OnDetached(); }

	void OnAttached(class Object* owner) override { IComponent::OnAttached(owner); g_renderShapes.emplace_back(owner, &m_renderData); }
	void OnDetached() override;
};