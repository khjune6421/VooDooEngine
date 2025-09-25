#include "Tree.h"

using namespace std;
using namespace DirectX;

vector<Object*> g_collidibleObjects;

void Tree::Update(float deltaTime)
{
	for (const auto& obj : g_collidibleObjects)
	{
		//if (obj == this) continue;

		const XMVECTOR distance = XMVectorSubtract(GetWorldPosition(), obj->GetWorldPosition());
		const float distanceLength = XMVectorGetX(XMVector3Length(distance));
		if (distanceLength < 5.0f)
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

// XMVector3Cross
// XMMatrixRotationAxis
// XMMatrixRotationNormal // this is faster if the axis is normalized value