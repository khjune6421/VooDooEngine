#pragma once

#include "Object.h"

class Scene
{
protected:
	std::vector<std::unique_ptr<Object>> m_objects;

public:
	Scene() = default;

	DirectX::XMFLOAT4 m_backgroundColor = { 0.5f, 0.5f, 0.5f, 1.0f };

	virtual void Update(float deltaTime) = 0;

	virtual void Raycast(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayEnd) {}
};