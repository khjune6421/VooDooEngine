#pragma once

#include "Object.h"

class WindMillWing : public Object
{
	friend class TestScene;

	Object m_windmillWing;
	bool m_rotationAngle = false;

public:
	WindMillWing(Shapes shape = Shapes::Triangle);
	~WindMillWing() override = default;

	void Update(float deltaTime) override;
};