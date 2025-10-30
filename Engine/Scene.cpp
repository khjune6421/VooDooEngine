#include "Scene.h"

#include "Renderer.h"
#include "Collider.h"

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

	CheckCollisions();

	SortRenderShapes();
}

void Scene::Render(Renderer* renderer) const
{
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	renderer->m_deviceContext->OMSetBlendState(renderer->m_blendStates[Renderer::AlphaToCoverage].Get(), nullptr, 0xffffffff); // It just works
	for (const auto& shape : m_renderShapes) shape->Render(renderer);

#ifdef _DEBUG
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	for (const auto& shape : m_renderShapes) shape->DebugRender(renderer);
#endif
}

void Scene::SortRenderShapes()
{
	if (!g_camera) return;

	XMVECTOR cameraPos = g_camera->m_cameraPosition;

	sort
	(
		m_renderShapes.begin(), m_renderShapes.end(),
		[&cameraPos](const Shape* a, const Shape* b)
		{
			XMVECTOR aPos = a->m_owner->GetWorldPosition();
			XMVECTOR bPos = b->m_owner->GetWorldPosition();
			float aDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(aPos, cameraPos)));
			float bDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(bPos, cameraPos)));
			return aDist < bDist;
		}
	);
}