#pragma once

#include "Engine.h"

class Windmill : public Object
{
	std::unique_ptr<Object> m_Wing = std::make_unique<Object>();

public:
	Windmill();

	void Update(float deltaTime) override;
};