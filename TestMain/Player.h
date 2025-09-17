#pragma once

#include "WindMillWing.h"

class Player : public Object
{
	friend class TestScene;

	std::unique_ptr<WindMillWing> m_windmillWing;

	float m_moveSpeed = 10.0f;

public:
	Player(Shapes shape = Shapes::Cube);
	~Player() override = default;

	void Update(float deltaTime) override;
};