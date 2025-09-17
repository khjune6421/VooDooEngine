#pragma once

#include "Object.h"

class Player : public Object
{
	std::unique_ptr<Object> m_windmillWing;

	float m_moveSpeed = 10.0f;

	public:
	Player(Shapes shape = Shapes::Cube);
	~Player() override = default;

	void Update(float deltaTime) override;
};