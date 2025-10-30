#include "Scene.h"

#include "Renderer.h"

using namespace DirectX;
using namespace std;

void Scene::Update(float deltaTime)
{
	for (const auto& object : m_objects) object->Update(deltaTime);

	SortRenderShapes();
}

void Scene::Render(Renderer* renderer) const
{
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (const auto& shape : g_renderShapes) shape->Render(renderer);
	//for (auto & g_renderShape : ranges::reverse_view(g_renderShapes)) g_renderShape->Render(renderer); // Reverse order

#ifdef _DEBUG
	renderer->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	for (const auto& shape : g_renderShapes) shape->DebugRender(this);
#endif
}

void Scene::SortRenderShapes()
{
	if (!g_camera) return;

	XMVECTOR cameraPos = g_camera->m_cameraPosition;

	sort
	(
		g_renderShapes.begin(), g_renderShapes.end(),
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