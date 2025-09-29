#pragma once

#include "WindMillWing.h"

class Player : public Object
{
	friend class TestScene;

	std::unique_ptr<WindMillWing> m_windmillWing;

	float m_moveSpeed = 10.0f;

	DirectX::XMVECTOR m_startPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR m_targetPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	bool m_isMovingToTarget = false;
	float m_moveToTargetTime = 0.0f;
	float m_moveToTargetElapsed = 0.0f;

public:
	Player(Shapes shape = Shapes::Cube);
	~Player() override = default;

	void Update(float deltaTime) override;

	void MoveToTarget(const DirectX::XMVECTOR& target);
};