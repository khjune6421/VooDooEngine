#include "pch.h"
#include "Scene.h"

#include "Renderer.h"
#include "Collider.h"
#include "Camera.h"
#include "Object.h"
#include "Shape.h"

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

void Scene::Update(float deltaTime)
{
	for (const auto& object : m_objects) object->Update(deltaTime);
	m_directionalLight.direction = XMVector3Normalize(m_directionalLight.direction);

	CheckCollisions();
	UpdateCamera();
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

void Scene::RenderShadowMap(Renderer* renderer)
{
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderer->m_deviceContext->OMSetBlendState(renderer->m_blendStates[Renderer::NoBlend].Get(), nullptr, 0xffffffff);

	renderer->m_deviceContext->VSSetShader(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"ShadowVertexShader"]].first.Get(), nullptr, 0);
	renderer->m_deviceContext->IASetInputLayout(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"ShadowVertexShader"]].second.Get());
	renderer->m_deviceContext->PSSetShader(renderer->m_pixelShaderMap[g_pixelShaderIdMap[L"ShadowPixelShader"]].Get(), nullptr, 0);

	MatrixConstBuffer shadowMatrixBuffer = m_matrixConstBuffer;

	XMVECTOR lightDirection = m_directionalLight.direction;
	XMVECTOR lightPosition = XMVectorScale(XMVectorNegate(lightDirection), 50.0f);
	XMVECTOR lightTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX lightViewMatrix = XMMatrixLookAtLH(lightPosition, lightTarget, lightUp);
	XMMATRIX lightProjectionMatrix = XMMatrixOrthographicLH(100.0f, 100.0f, 1.0f, 200.0f);

	shadowMatrixBuffer.view = XMMatrixTranspose(lightViewMatrix);
	shadowMatrixBuffer.projection = XMMatrixTranspose(lightProjectionMatrix);

	for (const auto& shape : m_renderShapes) shape->RenderShadowMap(renderer, &shadowMatrixBuffer);
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