#include "Tree.h"

using namespace std;
using namespace DirectX;

// These should be in some kind of collision manager later // very cursed
vector<Object*> g_collidibleObjects;
vector<Object*> g_attachableObjects;

void Tree::Update(float deltaTime)
{
	Rotate(XMVectorSet(0.0f, deltaTime, 0.0f, 0.0f));

	for (const auto& obj : g_collidibleObjects)
	{
		const XMVECTOR distance = XMVectorSubtract(GetWorldPosition(), obj->GetWorldPosition());
		const float distanceLength = XMVectorGetX(XMVector3Length(distance));
		if (distanceLength < 3.0f)
		{
			m_isDead = true;
			XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			XMVECTOR objFront = obj->GetWorldDirection(Directions::Forward);
			m_fallAngle = XMVector3Cross(up, objFront);
			break;
		}
	}

	if (m_isDead && !m_isAttached)
	{
		XMVECTOR upDir = GetWorldDirection(Directions::Up);
		if (XMVectorGetY(upDir) > 0.1f) Rotate(XMVectorScale(m_fallAngle, deltaTime));

		for (const auto& obj : g_attachableObjects)
		{
			const XMVECTOR distance = XMVectorSubtract(GetWorldPosition(), obj->GetWorldPosition());
			const float distanceLength = XMVectorGetX(XMVector3Length(distance));
			if (distanceLength < 3.0f)
			{
				m_isAttached = true;
				obj->AddChildViaWorldPosition(this);
				break;
			}
		}
	}
}

// XMVector3Cross
// XMMatrixRotationAxis
// XMMatrixRotationNormal // this is faster if the axis is normalized value