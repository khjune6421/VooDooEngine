#pragma once

#include "Engine.h"

extern std::vector<Object*> g_collidibleObjects;

class Tree : public Object
{
	bool m_isDead = false;

public:
	Tree() : Object(Shapes::Tree) {}
	~Tree() override = default;

	void Update(float deltaTime) override;
};