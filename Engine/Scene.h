#pragma once

#include <memory>

#include "Object.h"

class Scene
{
protected:
	// To manage objects lifetime // this is so fucked up
	std::vector<std::unique_ptr<Object>> m_objects;

	public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void Update(float deltaTime) = 0;
};