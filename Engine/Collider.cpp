#include "pch.h"
#include "Collider.h"

#include "Object.h"
#include "Scene.h"

using namespace std;
using namespace DirectX;

void Collider::CheckCollision(Collider* other)
{
	XMVECTOR posA = m_owner->GetWorldPosition();
	XMVECTOR posB = other->m_owner->GetWorldPosition();
	XMVECTOR delta = XMVectorSubtract(posA, posB);
	float distance = XMVectorGetX(XMVector3Length(delta));

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

	for (UINT group : m_collideTo) m_owner->m_scene->m_collidersMap[group].first.push_back(this);
	for (UINT group : m_collideFrom) m_owner->m_scene->m_collidersMap[group].second.push_back(this);
}

void Collider::OnDetached()
{
	for (UINT group : m_collideTo)
	{
		auto& vec = m_owner->m_scene->m_collidersMap[group].first;
		vec.erase(remove(vec.begin(), vec.end(), this), vec.end());
	}
	for (UINT group : m_collideFrom)
	{
		auto& vec = m_owner->m_scene->m_collidersMap[group].second;
		vec.erase(remove(vec.begin(), vec.end(), this), vec.end());
	}
	Component::OnDetached();
}