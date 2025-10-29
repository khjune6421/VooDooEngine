#include "Shape.h"

#include "Renderer.h"
#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

vector<Shape*> g_renderShapes;

Shape::Shape(const wstring& mesh, const wstring& vertexShader, const wstring& pixelShader, const vector<wstring>& textures)
{
	SetMesh(mesh);
	SetVertexShader(vertexShader);
	SetPixelShader(pixelShader);
	SetTextures(textures);
}

constexpr UINT stride = sizeof(Vertex);
constexpr UINT offset = 0;

void Shape::Render(Renderer* renderer) const
{
	renderer->m_deviceContext->IASetVertexBuffers(0, 1, renderer->m_shapeVertexBufferMap[m_meshId].first.GetAddressOf(), &stride, &offset);

	renderer->m_deviceContext->VSSetShader((renderer->m_vertexShaderMap[m_vertexShaderId]).first.Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetShader(renderer->m_pixelShaderMap[m_pixelShaderId].Get(), nullptr, 0);

	XMMATRIX worldMatrix = m_owner->GetWorldMatrix(); // This can be optimized

	Renderer::MatrixConstBuffer constBufferData = {};
	constBufferData.world = XMMatrixTranspose(worldMatrix);
	constBufferData.view = XMMatrixTranspose(Renderer::s_viewMatrix);
	constBufferData.projection = XMMatrixTranspose(Renderer::s_projectionMatrix);
	constBufferData.WVP = XMMatrixTranspose(worldMatrix * Renderer::s_viewMatrix * Renderer::s_projectionMatrix);
	constBufferData.normalMatrix = XMMatrixTranspose(m_owner->m_inverseScaleMatrix * worldMatrix);

	renderer->m_deviceContext->UpdateSubresource(renderer->m_constBuffers[Renderer::MatrixBuffer].Get(), 0, nullptr, &constBufferData, 0, 0);
	renderer->m_deviceContext->VSSetConstantBuffers(0, 1, renderer->m_constBuffers[Renderer::MatrixBuffer].GetAddressOf());

	renderer->m_deviceContext->IASetInputLayout(renderer->m_vertexShaderMap[m_vertexShaderId].second.Get());

	for (size_t i = 0; i < m_textureIds.size(); ++i)
	{
		renderer->m_deviceContext->PSSetShaderResources(static_cast<UINT>(i), 1, renderer->m_textureMap[m_textureIds[i]].GetAddressOf()); // This can be optimized further
	}

	renderer->m_deviceContext->Draw(renderer->m_shapeVertexBufferMap[m_meshId].second, 0);

	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	for (size_t i = 0; i < m_textureIds.size(); ++i)
	{
		renderer->m_deviceContext->PSSetShaderResources(static_cast<UINT>(i), 1, nullSRV);
	}
}

void Shape::SetMesh(const std::wstring& mesh)
{
	m_meshId = 0;
#ifdef _DEBUG
	if (g_meshIdMap.find(mesh) == g_meshIdMap.end()) MessageBoxW(nullptr, (L"Shape name not found: " + mesh).c_str(), L"Error", MB_OK);
#endif
	m_meshId = g_meshIdMap[mesh];
}

void Shape::SetVertexShader(const std::wstring& vertexShader)
{
	m_vertexShaderId = 0;
#ifdef _DEBUG
	if (g_vertexShaderIdMap.find(vertexShader) == g_vertexShaderIdMap.end()) MessageBoxW(nullptr, (L"Vertex shader not found: " + vertexShader).c_str(), L"Error", MB_OK);
#endif
	m_vertexShaderId = g_vertexShaderIdMap[vertexShader];
}

void Shape::SetPixelShader(const std::wstring& pixelShader)
{
	m_pixelShaderId = 0;
#ifdef _DEBUG
	if (g_pixelShaderIdMap.find(pixelShader) == g_pixelShaderIdMap.end()) MessageBoxW(nullptr, (L"Pixel shader not found: " + pixelShader).c_str(), L"Error", MB_OK);
#endif
	m_pixelShaderId = g_pixelShaderIdMap[pixelShader];
}

void Shape::SetTextures(const std::vector<std::wstring>& textures)
{
	m_textureIds.clear();
	for (const auto& texture : textures)
	{
#ifdef _DEBUG
		if (g_textureIdMap.find(texture) == g_textureIdMap.end()) MessageBoxW(nullptr, (L"Texture not found: " + texture).c_str(), L"Error", MB_OK);
#endif
		m_textureIds.push_back(g_textureIdMap[texture]);
	}
}

void Shape::OnAttached(Object* owner)
{
	Component::OnAttached(owner);
	g_renderShapes.emplace_back(this);
}

void Shape::OnDetached()
{
	auto it = find(g_renderShapes.begin(), g_renderShapes.end(), this);
	if (it != g_renderShapes.end()) g_renderShapes.erase(it);
	Component::OnDetached();
}