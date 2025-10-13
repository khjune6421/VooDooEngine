#pragma once

#include "WindMillWing.h"

class Player : public AnimationObject
{
	friend class TestScene;

	std::unique_ptr<WindMillWing> m_windmillWing = nullptr;

	float m_moveSpeed = 10.0f;

	DirectX::XMVECTOR m_startPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR m_targetPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR m_startRotation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR m_targetRotation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	bool m_isMovingToTarget = false;
	float m_moveToTargetTime = 0.0f;
	float m_moveToTargetElapsed = 0.0f;

	float ATime = 0.0f;

public:
	Player(const std::vector<std::wstring>& shapes) : AnimationObject(shapes, L"TripleVertexShader") {}
	~Player() override = default;

	void Update(float deltaTime) override;

	void MoveToTarget(const DirectX::XMVECTOR& target, const DirectX::XMVECTOR& targetRotation);
};