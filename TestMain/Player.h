#pragma once

#include "Engine.h"

class Player : public Object
{
	float m_moveSpeed = 10.0f;

public:
	Player();

	void Update(float deltaTime) override;
};