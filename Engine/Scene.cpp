#include "Scene.h"

using namespace DirectX;
using namespace std;

void Scene::Update(float deltaTime)
{
	for (const auto& object : m_objects) object->Update(deltaTime);

	SortRenderShapes();
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