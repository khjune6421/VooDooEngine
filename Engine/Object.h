#pragma once

#include "Render.h"

class Object
{
	friend class Scene;
	// is this a good idea?
	friend class Render;

	Shapes m_shape = Shapes::Triangle;
	Transform m_transform;

	bool m_isActive = true;

public:
	Object(Shapes shape = Shapes::Triangle);
	~Object();

	Transform& GetTransform() { return m_transform; }
};