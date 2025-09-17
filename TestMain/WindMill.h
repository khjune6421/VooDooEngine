#pragma once

#include "Object.h"

class WindMill : public Object
{
	std::unique_ptr<Object> m_windmillWing;

	float m_moveSpeed = 10.0f;

public:
	WindMill(Shapes shape = Shapes::Tetrahedron);
	~WindMill() override = default;

	void Update(float deltaTime) override;
};