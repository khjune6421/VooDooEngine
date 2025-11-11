#include "pch.h"
#include "Collider.h"

#include "Object.h"
#include "Scene.h"

using namespace std;
using namespace DirectX;

void Collider::CheckCollision(Collider* other)
{
	const XMVECTOR posA = m_owner->GetWorldPosition();
	const XMVECTOR posB = other->m_owner->GetWorldPosition();
	const XMVECTOR delta = XMVectorSubtract(posA, posB);
	const float distance = XMVectorGetX(XMVector3Length(delta));

	if (distance <= (m_radius + other->m_radius))
	{
		if (!m_isColliding)
		{
			m_isCollided = true;
			m_isColliding = true;
			m_collidedPosition = posB;
		}
		if (!other->m_isColliding)
		{
			other->m_isCollided = true;
			other->m_isColliding = true;
			other->m_collidedPosition = posA;
		}
	}
	else
	{
		m_isColliding = false;
		other->m_isColliding = false;
	}
}

void Collider::OnAttached(Object* owner)
{
	Component::OnAttached(owner);

	for (const UINT group : m_collideTo) m_owner->m_scene->m_colliderMap[group].first.push_back(this);
	for (const UINT group : m_collideFrom) m_owner->m_scene->m_colliderMap[group].second.push_back(this);
}

void Collider::OnDetached()
{
	for (const UINT group : m_collideTo)
	{
		auto& vec = m_owner->m_scene->m_colliderMap[group].first;
		vec.erase(remove(vec.begin(), vec.end(), this), vec.end());
	}
	for (const UINT group : m_collideFrom)
	{
		auto& vec = m_owner->m_scene->m_colliderMap[group].second;
		vec.erase(remove(vec.begin(), vec.end(), this), vec.end());
	}
	Component::OnDetached();
}