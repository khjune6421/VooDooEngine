#pragma once

#include "Engine.h"
#include "Bullet.h"

class Player : public Object
{
	float m_moveSpeed = 10.0f;

	std::deque<std::unique_ptr<Bullet>> m_bullets = {};

public:
	Player(Scene* scene) : Object(scene) {}

	void Update(float deltaTime) override;
};