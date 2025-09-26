#pragma once

#include "Engine.h"

extern std::vector<Object*> g_collidibleObjects;
extern std::vector<Object*> g_attachableObjects;

class Tree : public Object
{
	bool m_isDead = false;
	bool m_isAttached = false;

	DirectX::XMVECTOR m_fallAngle = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

public:
	Tree() : Object(Shapes::Tree) {}
	~Tree() override = default;

	void Update(float deltaTime) override;
};