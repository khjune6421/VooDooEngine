#include "pch.h"
#include "Scene.h"

#include "Renderer.h"
#include "Collider.h"
#include "Camera.h"
#include "Object.h"
#include "Light.h"

using namespace DirectX;
using namespace std;

void Scene::CheckCollisions()
{
	for (auto& colliderGroup : m_collidersMap)
	{
		for (auto& colliderA : colliderGroup.second.first)
		{
			for (auto& colliderB : colliderGroup.second.second)
			{
				if (colliderA != colliderB) colliderA->CheckCollision(colliderB);
			}
		}
	}
}

void Scene::UpdateCamera()
{
	if (!m_mainCamera) return;

	m_mainCameraPosition = m_mainCamera->GetPosition();
	m_matrixConstBuffer.view = XMMatrixTranspose(m_mainCamera->GetViewMatrix());
	m_matrixConstBuffer.projection = XMMatrixTranspose(m_mainCamera->GetProjectionMatrix());

	sort
	(
		m_renderShapes.begin(), m_renderShapes.end(),
		[this](const Shape* a, const Shape* b)
		{
			XMVECTOR aPos = a->m_owner->GetWorldPosition();
			XMVECTOR bPos = b->m_owner->GetWorldPosition();
			float aDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(aPos, m_mainCameraPosition)));
			float bDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(bPos, m_mainCameraPosition)));
			return aDist < bDist;
		}
	);
}

void Scene::UpdateLight(Renderer* renderer)
{
	PointLightArrayConstBuffer pointLightBufferData = {};
	pointLightBufferData.numPointLights = static_cast<UINT>(m_pointLights.size());
	for (UINT i = 0; i < pointLightBufferData.numPointLights && i < MAX_POINT_LIGHTS; ++i) pointLightBufferData.pointLights[i] = m_pointLights[i]->GetLightData();
	renderer->m_deviceContext->UpdateSubresource(renderer->m_constBuffers[Renderer::PointLightBuffer].Get(), 0, nullptr, &pointLightBufferData, 0, 0);
	renderer->m_deviceContext->PSSetConstantBuffers(0, 1, renderer->m_constBuffers[Renderer::PointLightBuffer].GetAddressOf());

	CreateShadowMap(renderer);
}

#define comPtr Microsoft::WRL::ComPtr

void Scene::CreateShadowMap(Renderer* renderer)
{
	comPtr<ID3D11RenderTargetView> originalRTV;
	comPtr<ID3D11DepthStencilView> originalDSV;
	renderer->m_deviceContext->OMGetRenderTargets(1, originalRTV.GetAddressOf(), originalDSV.GetAddressOf());

	D3D11_VIEWPORT originalViewport;
	UINT numViewports = 1;
	renderer->m_deviceContext->RSGetViewports(&numViewports, &originalViewport);

	D3D11_VIEWPORT shadowViewport = {};
	shadowViewport.TopLeftX = 0.0f;
	shadowViewport.TopLeftY = 0.0f;
	shadowViewport.Width = static_cast<FLOAT>(Renderer::SHADOW_MAP_SIZE);
	shadowViewport.Height = static_cast<FLOAT>(Renderer::SHADOW_MAP_SIZE);
	shadowViewport.MinDepth = 0.0f;
	shadowViewport.MaxDepth = 1.0f;

	renderer->m_deviceContext->RSSetViewports(1, &shadowViewport);
	renderer->m_deviceContext->IASetInputLayout(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"DepthOnlyVertexShader"]].second.Get());
	renderer->m_deviceContext->VSSetShader(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"DepthOnlyVertexShader"]].first.Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetShader(renderer->m_pixelShaderMap[g_pixelShaderIdMap[L"DepthOnlyPixelShader"]].Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetSamplers(0, 1, renderer->m_samplers[Renderer::DefaultSampler].GetAddressOf());

	for (UINT i = 0; i < static_cast<UINT>(m_pointLights.size()) && i < MAX_POINT_LIGHTS; ++i) m_pointLights[i]->CreateShadowMap(renderer, 6 * i);

	renderer->m_deviceContext->OMSetRenderTargets(1, originalRTV.GetAddressOf(), originalDSV.Get());
	renderer->m_deviceContext->RSSetViewports(1, &originalViewport);
}

#undef comPtr

void Scene::RenderShadows(Renderer* renderer, MatrixConstBuffer* lightMatrixBuffer)
{
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const auto& shape : m_renderShapes) shape->RenderShadow(renderer, lightMatrixBuffer);
}

void Scene::Update(float deltaTime)
{
	for (const auto& object : m_objects) object->Update(deltaTime);
	m_directionalLight.direction = XMVector3Normalize(m_directionalLight.direction);

	CheckCollisions();
	UpdateCamera();
}

void Scene::PreRender(Renderer* renderer)
{
	UpdateLight(renderer);
}

void Scene::Render(Renderer* renderer)
{
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	renderer->m_deviceContext->OMSetBlendState(renderer->m_blendStates[Renderer::AlphaToCoverage].Get(), nullptr, 0xffffffff); // It just works // but only if it is sorted
	for (const auto& shape : m_renderShapes) shape->Render(renderer, &m_matrixConstBuffer);

#ifdef _DEBUG
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	for (const auto& shape : m_renderShapes) shape->DebugRender(renderer, &m_matrixConstBuffer);
#endif
}