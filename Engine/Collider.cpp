#include "Collider.h"

#include "Object.h"
#include "Scene.h"

using namespace std;
using namespace DirectX;

//void Collider::CheckCollision(const Collider* other) const
//{
//	XMVECTOR posA = m_owner->GetWorldPosition();
//	XMVECTOR posB = other->m_owner->GetWorldPosition();
//	XMVECTOR delta = XMVectorSubtract(posA, posB);
//	float distance = XMVectorGetX(XMVector3Length(delta));
//
//	return distance <= (m_radius + other->m_radius);
//}

void Collider::CheckCollision(Collider* other)
{
	XMVECTOR posA = m_owner->GetWorldPosition();
	XMVECTOR posB = other->m_owner->GetWorldPosition();
	XMVECTOR delta = XMVectorSubtract(posA, posB);
	float distance = XMVectorGetX(XMVector3Length(delta));

	m_isColliding = distance <= (m_radius + other->m_radius);
	other->m_isColliding = m_isColliding;
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