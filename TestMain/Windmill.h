#pragma once

#include "pch.h"

class Windmill : public Object
{
	std::unique_ptr<Object> m_Wing = nullptr;

public:
	Windmill(Scene* scene);

	void Update(float deltaTime) override;
};