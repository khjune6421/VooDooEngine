#include "Tree.h"

using namespace std;
using namespace DirectX;

void Tree::Update(float deltaTime)
{
	if (!m_isDead && m_collider->IsCollided())
	{
		m_isDead = true;
		XMVECTOR position = GetWorldPosition();
		XMVECTOR collidedPos = m_collider->GetCollidedPosition();
		position = XMVectorSetY(position, 0.0f);
		collidedPos = XMVectorSetY(collidedPos, 0.0f);
		XMVECTOR direction = XMVectorSubtract(position, collidedPos);
		direction = XMVector3Normalize(direction);
		m_fallDirection = XMVector3Cross(-direction, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}

	if (m_isDead)
	{
		XMVECTOR upDir = GetWorldDirection(Directions::Up);
		if (XMVectorGetY(upDir) > 0.1f) Rotate(XMVectorScale(m_fallDirection, deltaTime));
	}
}