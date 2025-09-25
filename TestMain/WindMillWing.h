#pragma once

#include "Engine.h"

class WindMillWing : public Object
{
	friend class TestScene;

	//std::unique_ptr<Object> m_wing[3];
	std::vector<std::unique_ptr<Object>> m_wing;

public:
	WindMillWing(UINT wingAmount = 3);
	~WindMillWing() override = default;

	void Update(float deltaTime) override;
};