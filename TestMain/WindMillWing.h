#pragma once

#include "Object.h"

class WindMillWing : public Object
{
	friend class TestScene;

	std::unique_ptr<Object> m_wing[3];

public:
	WindMillWing();
	~WindMillWing() override = default;

	void Update(float deltaTime) override;
};