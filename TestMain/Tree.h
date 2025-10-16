#pragma once

#include "Engine.h"

class Tree : public Object
{
	bool m_isDead = false;
	bool m_isAttached = false;

	DirectX::XMVECTOR m_fallAngle = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

public:
	void Update(float deltaTime) override;
};