#include "Tree.h"

using namespace std;
using namespace DirectX;

vector<Object*> g_collidibleObjects;

void Tree::Update(float deltaTime)
{
	for (const auto& obj : g_collidibleObjects)
	{
		if (obj == this) continue;
		const XMVECTOR DISTANCE = XMVectorSubtract(GetWorldPosition(), obj->GetWorldPosition());
		const float DISTANCE_LENGTH = XMVectorGetX(XMVector3Length(DISTANCE));
		if (DISTANCE_LENGTH < 5.0f)
		{
			m_isDead = true;
			break;
		}
	}

	if (m_isDead && m_rotation.m128_f32[0] < XM_PIDIV2)
	{
		Rotate(XMVECTOR{ 2.0f * deltaTime, 0.0f, 0.0f, 0.0f });
	}
}