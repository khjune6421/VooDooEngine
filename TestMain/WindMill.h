#pragma once

#include "WindMillWing.h"

class WindMill : public Object
{
	friend class TestScene;

	std::unique_ptr<WindMillWing> m_windmillWing;

	float m_moveSpeed = 10.0f;

public:
	WindMill(Shapes shape = Shapes::Tetrahedron, UINT wingAmount = 3);
	~WindMill() override = default;

	void Update(float deltaTime) override;
};