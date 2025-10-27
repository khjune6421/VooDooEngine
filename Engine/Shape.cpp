#include "Shape.h"

using namespace std;

vector<pair<Object*, ShapeData*>> g_renderShapes;

Shape::Shape(const wstring& mesh, const wstring& vertexShader, const wstring& pixelShader, const vector<wstring>& textures)
{
	SetMesh(mesh);
	SetVertexShader(vertexShader);
	SetPixelShader(pixelShader);
	SetTextures(textures);
}

void Shape::SetMesh(const std::wstring& mesh)
{
	m_renderData.meshId = 0;
#ifdef _DEBUG
	if (g_meshIdMap.find(mesh) == g_meshIdMap.end()) MessageBoxW(nullptr, (L"Shape name not found: " + mesh).c_str(), L"Error", MB_OK);
#endif
	m_renderData.meshId = g_meshIdMap[mesh];
}

void Shape::SetVertexShader(const std::wstring& vertexShader)
{
	m_renderData.vertexShaderId = 0;
#ifdef _DEBUG
	if (g_vertexShaderIdMap.find(vertexShader) == g_vertexShaderIdMap.end()) MessageBoxW(nullptr, (L"Vertex shader not found: " + vertexShader).c_str(), L"Error", MB_OK);
#endif
	m_renderData.vertexShaderId = g_vertexShaderIdMap[vertexShader];
}

void Shape::SetPixelShader(const std::wstring& pixelShader)
{
	m_renderData.pixelShaderId = 0;
#ifdef _DEBUG
	if (g_pixelShaderIdMap.find(pixelShader) == g_pixelShaderIdMap.end()) MessageBoxW(nullptr, (L"Pixel shader not found: " + pixelShader).c_str(), L"Error", MB_OK);
#endif
	m_renderData.pixelShaderId = g_pixelShaderIdMap[pixelShader];
}

void Shape::SetTextures(const std::vector<std::wstring>& textures)
{
	m_renderData.textureIds.clear();
	for (const auto& texture : textures)
	{
#ifdef _DEBUG
		if (g_textureIdMap.find(texture) == g_textureIdMap.end()) MessageBoxW(nullptr, (L"Texture not found: " + texture).c_str(), L"Error", MB_OK);
#endif
		m_renderData.textureIds.push_back(g_textureIdMap[texture]);
	}
}

void Shape::OnDetached()
{
	auto it = remove_if(g_renderShapes.begin(), g_renderShapes.end(), [this](const pair<Object*, ShapeData*>& pair) { return pair.second == &m_renderData; });
	g_renderShapes.erase(it, g_renderShapes.end());
}