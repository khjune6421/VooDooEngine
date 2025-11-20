#pragma once
#include "Object.h"

class Bullet : public Object
{
	float m_lifeTime = 3.0f;

public:
	Bullet(Scene* scene);
	void Update(float deltaTime) override;

	bool IsExpired() const { return m_lifeTime <= 0.0f; }
};