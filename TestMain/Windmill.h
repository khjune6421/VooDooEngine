#pragma once

#include "Engine.h"

class Windmill : public Object
{
	std::unique_ptr<Object> m_Wing = std::make_unique<Object>();

public:
	Windmill();
	~Windmill() = default;

	void Update(float deltaTime) override;
};