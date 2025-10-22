#include "Shape.h"

using namespace std;

vector<pair<Object*, ShapeData*>> g_renderShapes;

Shape::Shape(const wstring& mesh, const wstring& vertexShader, const wstring& pixelShader, const wstring& texture)
{
#ifdef _DEBUG
	if (g_meshIdMap.find(mesh) == g_meshIdMap.end()) MessageBoxW(nullptr, (L"Shape name not found: " + mesh).c_str(), L"Error", MB_OK);
#endif
	m_renderData.meshId = g_meshIdMap[mesh];

#ifdef _DEBUG
	if (g_vertexShaderIdMap.find(vertexShader) == g_vertexShaderIdMap.end()) MessageBoxW(nullptr, (L"Vertex shader not found: " + vertexShader).c_str(), L"Error", MB_OK);
#endif
	m_renderData.vertexShaderId = g_vertexShaderIdMap[vertexShader];

#ifdef _DEBUG
	if (g_pixelShaderIdMap.find(pixelShader) == g_pixelShaderIdMap.end()) MessageBoxW(nullptr, (L"Pixel shader not found: " + pixelShader).c_str(), L"Error", MB_OK);
#endif
	m_renderData.pixelShaderId = g_pixelShaderIdMap[pixelShader];

#ifdef _DEBUG
	if (g_textureIdMap.find(texture) == g_textureIdMap.end()) MessageBoxW(nullptr, (L"Texture not found: " + texture).c_str(), L"Error", MB_OK);
#endif
	m_renderData.textureIds.push_back(g_textureIdMap[texture]);
}

void Shape::OnDetached()
{
	auto it = remove_if(g_renderShapes.begin(), g_renderShapes.end(), [this](const pair<Object*, ShapeData*>& pair) { return pair.second == &m_renderData; });
	g_renderShapes.erase(it, g_renderShapes.end());
}