#pragma once

#include "Object.h"

class Scene
{
protected:
	std::vector<std::unique_ptr<Object>> m_objects;

public:
	Scene() = default;

	virtual void Update(float deltaTime) = 0;

	virtual void Raycast(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayEnd) {}
};