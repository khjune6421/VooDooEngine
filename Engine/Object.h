#pragma once

#include "Render.h"

extern std::vector<Object*> g_objects;

class Object
{
	Transform m_transform;

public:
	Object();
	~Object();

	Transform& GetTransform() { return m_transform; }
};