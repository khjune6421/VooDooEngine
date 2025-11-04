#include "pch.h"
#include "Shape.h"

#include "Renderer.h"
#include "Object.h"
#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

Shape::Shape(const wstring& mesh, const wstring& vertexShader, const wstring& pixelShader, const wstring& texture, const wstring& normalMap)
{
	SetMesh(mesh);
	SetVertexShader(vertexShader);
	SetPixelShader(pixelShader);
	SetTexture(texture);
	SetNormalMap(normalMap);
}

constexpr UINT stride = sizeof(Vertex);
constexpr UINT offset = 0;

void Shape::Render(Renderer* renderer, MatrixConstBuffer* matrixBuffer)
{
	renderer->m_deviceContext->IASetVertexBuffers(0, 1, renderer->m_meshVertexBufferMap[m_meshId].first.GetAddressOf(), &stride, &offset);

	renderer->m_deviceContext->VSSetShader((renderer->m_vertexShaderMap[m_vertexShaderId]).first.Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetShader(renderer->m_pixelShaderMap[m_pixelShaderId].Get(), nullptr, 0);

	matrixBuffer->world = XMMatrixTranspose(m_owner->GetWorldMatrix());
	matrixBuffer->WVP = matrixBuffer->projection * matrixBuffer->view * matrixBuffer->world;
	matrixBuffer->normalMatrix = XMMatrixTranspose(m_owner->m_inverseScaleMatrix * m_owner->GetWorldMatrix());

	renderer->m_deviceContext->UpdateSubresource(renderer->m_constBuffers[Renderer::MatrixBuffer].Get(), 0, nullptr, matrixBuffer, 0, 0);
	renderer->m_deviceContext->VSSetConstantBuffers(0, 1, renderer->m_constBuffers[Renderer::MatrixBuffer].GetAddressOf());

	renderer->m_deviceContext->IASetInputLayout(renderer->m_vertexShaderMap[m_vertexShaderId].second.Get());

	renderer->m_deviceContext->PSSetShaderResources(1, 1, renderer->m_textureMap[m_textureId].GetAddressOf());
	renderer->m_deviceContext->PSSetShaderResources(2, 1, renderer->m_textureMap[m_normalMapId].GetAddressOf());

	renderer->m_deviceContext->Draw(renderer->m_meshVertexBufferMap[m_meshId].second, 0);
}

void Shape::RenderShadow(Renderer* renderer, MatrixConstBuffer* lightMatrixBuffer)
{
	renderer->m_deviceContext->IASetVertexBuffers(0, 1, renderer->m_meshVertexBufferMap[m_meshId].first.GetAddressOf(), &stride, &offset);

	lightMatrixBuffer->world = XMMatrixTranspose(m_owner->GetWorldMatrix());
	lightMatrixBuffer->WVP = lightMatrixBuffer->projection * lightMatrixBuffer->view * lightMatrixBuffer->world;

	renderer->m_deviceContext->UpdateSubresource(renderer->m_constBuffers[Renderer::MatrixBuffer].Get(), 0, nullptr, lightMatrixBuffer, 0, 0);
	renderer->m_deviceContext->VSSetConstantBuffers(0, 1, renderer->m_constBuffers[Renderer::MatrixBuffer].GetAddressOf());
	renderer->m_deviceContext->Draw(renderer->m_meshVertexBufferMap[m_meshId].second, 0);
}

#ifdef _DEBUG
void Shape::DebugRender(Renderer* renderer, struct MatrixConstBuffer* matrixBuffer)
{
	renderer->m_deviceContext->IASetVertexBuffers(0, 1, renderer->m_meshVertexBufferMap[m_meshId].first.GetAddressOf(), &stride, &offset);
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	renderer->m_deviceContext->VSSetShader(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"VSShowNormal"]].first.Get(), nullptr, 0);
	renderer->m_deviceContext->GSSetShader(renderer->m_geometryShaderMap[g_geometryShaderIdMap[L"GSShowNormal"]].Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetShader(renderer->m_pixelShaderMap[g_pixelShaderIdMap[L"PSShowNormal"]].Get(), nullptr, 0);

	matrixBuffer->world = XMMatrixTranspose(m_owner->GetWorldMatrix());
	matrixBuffer->normalMatrix = XMMatrixTranspose(m_owner->m_inverseScaleMatrix * m_owner->GetWorldMatrix());

	renderer->m_deviceContext->UpdateSubresource(renderer->m_constBuffers[Renderer::MatrixBuffer].Get(), 0, nullptr, matrixBuffer, 0, 0);
	renderer->m_deviceContext->VSSetConstantBuffers(0, 1, renderer->m_constBuffers[Renderer::MatrixBuffer].GetAddressOf());
	renderer->m_deviceContext->GSSetConstantBuffers(0, 1, renderer->m_constBuffers[Renderer::MatrixBuffer].GetAddressOf());

	renderer->m_deviceContext->IASetInputLayout(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"VSShowNormal"]].second.Get());

	renderer->m_deviceContext->Draw(renderer->m_meshVertexBufferMap[m_meshId].second, 0);

	renderer->m_deviceContext->GSSetShader(nullptr, nullptr, 0);
}
#endif

void Shape::SetMesh(const std::wstring& mesh)
{
	m_meshId = 0;
#ifdef _DEBUG
	if (g_meshIdMap.find(mesh) == g_meshIdMap.end()) MessageBoxW(nullptr, (L"Shape name not found: " + mesh).c_str(), L"Error", MB_OK);
#endif
	m_meshId = g_meshIdMap[mesh];
}

void Shape::SetVertexShader(const wstring& vertexShader)
{
	m_vertexShaderId = 0;
#ifdef _DEBUG
	if (g_vertexShaderIdMap.find(vertexShader) == g_vertexShaderIdMap.end()) MessageBoxW(nullptr, (L"Vertex shader not found: " + vertexShader).c_str(), L"Error", MB_OK);
#endif
	m_vertexShaderId = g_vertexShaderIdMap[vertexShader];
}

void Shape::SetPixelShader(const wstring& pixelShader)
{
	m_pixelShaderId = 0;
#ifdef _DEBUG
	if (g_pixelShaderIdMap.find(pixelShader) == g_pixelShaderIdMap.end()) MessageBoxW(nullptr, (L"Pixel shader not found: " + pixelShader).c_str(), L"Error", MB_OK);
#endif
	m_pixelShaderId = g_pixelShaderIdMap[pixelShader];
}

void Shape::SetTexture(const wstring& texture)
{
	m_textureId = 0;
#ifdef _DEBUG
	if (g_textureIdMap.find(texture) == g_textureIdMap.end()) MessageBoxW(nullptr, (L"Texture not found: " + texture).c_str(), L"Error", MB_OK);
#endif
	m_textureId = g_textureIdMap[texture];
}

void Shape::SetNormalMap(const std::wstring& normalMap)
{
	m_normalMapId = 0;
#ifdef _DEBUG
	if (g_textureIdMap.find(normalMap) == g_textureIdMap.end()) MessageBoxW(nullptr, (L"Normal map not found: " + normalMap).c_str(), L"Error", MB_OK);
#endif
	m_normalMapId = g_textureIdMap[normalMap];
}

void Shape::OnAttached(Object* owner)
{
	Component::OnAttached(owner);

	owner->m_scene->m_renderShapes.emplace_back(this);
}

void Shape::OnDetached()
{
	auto it = find(m_owner->m_scene->m_renderShapes.begin(), m_owner->m_scene->m_renderShapes.end(), this);
	if (it != m_owner->m_scene->m_renderShapes.end()) m_owner->m_scene->m_renderShapes.erase(it);

	Component::OnDetached();
}