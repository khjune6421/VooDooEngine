#include "pch.h"
#include "Scene.h"

#include "Renderer.h"
#include "Collider.h"
#include "Camera.h"
#include "Object.h"

using namespace DirectX;
using namespace std;

void Scene::CheckCollisions()
{
	for (auto& colliderGroup : m_colliderMap)
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
	m_pointLightBufferData.numPointLights = static_cast<UINT>(m_pointLights.size());
	for (UINT i = 0; i < m_pointLightBufferData.numPointLights && i < MAX_POINT_LIGHTS; ++i) m_pointLightBufferData.pointLights[i] = m_pointLights[i]->GetLightData();

	UpdateShadowMap(renderer);
}

void Scene::UpdateShadowMap(Renderer* renderer)
{
	com_ptr<ID3D11RenderTargetView> originalRTV;
	com_ptr<ID3D11DepthStencilView> originalDSV;
	renderer->m_deviceContext->OMGetRenderTargets(1, originalRTV.GetAddressOf(), originalDSV.GetAddressOf());

	D3D11_VIEWPORT originalViewport;
	UINT numViewports = 1;
	renderer->m_deviceContext->RSGetViewports(&numViewports, &originalViewport);

	D3D11_RECT originalScissorRect;
	UINT numScissorRects = 1;
	renderer->m_deviceContext->RSGetScissorRects(&numScissorRects, &originalScissorRect);

	constexpr D3D11_VIEWPORT shadowViewport =
	{
		0.0f, 0.0f,
		static_cast<FLOAT>(Renderer::SHADOW_MAP_SIZE),
		static_cast<FLOAT>(Renderer::SHADOW_MAP_SIZE),
		0.0f, 1.0f
	};
	renderer->m_deviceContext->RSSetViewports(1, &shadowViewport);

	constexpr D3D11_RECT shadowScissorRect =
	{
		0, 0,
		static_cast<LONG>(Renderer::SHADOW_MAP_SIZE),
		static_cast<LONG>(Renderer::SHADOW_MAP_SIZE)
	};
	renderer->m_deviceContext->RSSetScissorRects(1, &shadowScissorRect);

	renderer->m_deviceContext->IASetInputLayout(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"DepthOnlyVertexShader"]].second.Get());
	renderer->m_deviceContext->VSSetShader(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"DepthOnlyVertexShader"]].first.Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetShader(renderer->m_pixelShaderMap[g_pixelShaderIdMap[L"DepthOnlyPixelShader"]].Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetSamplers(1, 1, renderer->m_samplers[Renderer::DefaultSampler].GetAddressOf());

	for (UINT i = 0; i < static_cast<UINT>(m_pointLights.size()) && i < MAX_POINT_LIGHTS; ++i) m_pointLights[i]->CreateShadowMap(renderer, 6 * i);

	renderer->m_deviceContext->OMSetRenderTargets(1, originalRTV.GetAddressOf(), originalDSV.Get());
	renderer->m_deviceContext->RSSetViewports(1, &originalViewport);
	renderer->m_deviceContext->RSSetScissorRects(1, &originalScissorRect);
}

void Scene::RenderShadows(Renderer* renderer, MatrixConstBuffer* lightMatrixBuffer)
{
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const auto& shape : m_renderShapes) shape->RenderShadow(renderer, lightMatrixBuffer);
}

void Scene::Update(float deltaTime)
{
	for (const auto& object : m_objects) object->Update(deltaTime);

	CheckCollisions();
	UpdateCamera();
}

void Scene::PreRender(Renderer* renderer)
{
	if (renderer->m_shouldUpdateLights) UpdateLight(renderer);
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