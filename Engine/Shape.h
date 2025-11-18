#pragma once
#include "pch.h"
#include "Component.h"

extern std::unordered_map<std::wstring, UINT> g_meshIdMap;
extern std::unordered_map<std::wstring, UINT> g_vertexShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_geometryShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_pixelShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_textureIdMap;

class Shape : public Component
{
	UINT m_meshId = 0;
	UINT m_vertexShaderId = 0;
	UINT m_pixelShaderId = 0;

	UINT m_textureId = 0;
	UINT m_normalMapId = 0;

	UINT m_stencilRef = 0;

public:
	Shape
	(
		const std::wstring& mesh,
		const std::wstring& vertexShader = L"VertexShader",
		const std::wstring& pixelShader = L"PixelShader",

		const std::wstring& texture = L"NoTexture",
		const std::wstring& normalMap = L"NoNormal"
	);

	void Render(class Renderer* renderer, struct MatrixConstBuffer* matrixBuffer);
	void RenderShadow(class Renderer* renderer, struct MatrixConstBuffer* lightMatrixBuffer);
#ifdef _DEBUG
	void DebugRender(class Renderer* renderer, struct MatrixConstBuffer* matrixBuffer);
#endif

	void SetMesh(const std::wstring& mesh);
	void SetVertexShader(const std::wstring& vertexShader);
	void SetPixelShader(const std::wstring& pixelShader);

	void SetTexture(const std::wstring& texture);
	void SetNormalMap(const std::wstring& normalMap);

	void SetStencilRef(UINT stencilRef) { m_stencilRef = stencilRef; }
	UINT GetStencilRef() const { return m_stencilRef; }

	void OnAttached(Object* owner) override;
	void OnDetached() override;
};