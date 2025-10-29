#pragma once

#include "UtilityHeaders.h"
#include "DirectXLib.h"

#include "Component.h"

extern std::unordered_map<std::wstring, UINT> g_meshIdMap;
extern std::unordered_map<std::wstring, UINT> g_vertexShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_geometryShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_pixelShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_textureIdMap;

class Renderer;

class Shape : public Component
{
	UINT m_meshId = 0;
	UINT m_vertexShaderId = 0;
	UINT m_pixelShaderId = 0;

	std::vector<UINT> m_textureIds = {};

public:
	Shape
	(
		const std::wstring& mesh,
		const std::wstring& vertexShader = L"VertexShader",
		const std::wstring& pixelShader = L"PixelShader",
		const std::vector<std::wstring>& textures = { L"NoTexture", L"NoNormal" }
	);

	void Render(Renderer* renderer) const;

	void SetMesh(const std::wstring& mesh);
	void SetVertexShader(const std::wstring& vertexShader);
	void SetPixelShader(const std::wstring& pixelShader);
	void SetTextures(const std::vector<std::wstring>& textures);

	void OnAttached(class Object* owner) override;
	void OnDetached() override;
};