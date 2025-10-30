#pragma once

#include "UtilityHeaders.h"

#include "Component.h"

class Collider : public Component
{
	float m_radius = 1.0f;
	std::vector<UINT> m_collideTo = {};
	std::vector<UINT> m_collideFrom = {};

	bool m_isColliding = false;

public:
	Collider
	(
		float radius = 1.0f,
		std::vector<UINT> collideTo = {},
		std::vector<UINT> collideFrom = {}
	) : m_radius(radius),
		m_collideTo(collideTo),
		m_collideFrom(collideFrom) {}

	void CheckCollision(Collider* other);
	bool IsColliding() const { return m_isColliding; }

	void GetRadius(float& radius) const { radius = m_radius; }
	void SetRadius(float radius) { m_radius = radius; }
	void GetCollideTo(std::vector<UINT>& groups) const { groups = m_collideTo; }
	void SetCollideTo(const std::vector<UINT>& groups) { m_collideTo = groups; }
	void GetCollideFrom(std::vector<UINT>& groups) const { groups = m_collideFrom; }
	void SetCollideFrom(const std::vector<UINT>& groups) { m_collideFrom = groups; }

	void OnAttached(class Object* owner) override;
	void OnDetached() override;
};