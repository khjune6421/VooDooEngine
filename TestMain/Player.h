#pragma once

#include "WindMillWing.h"

class Player : public Object
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

	float m_val1 = 0.0f;
	float m_val2 = 0.0f;
	float m_val3 = 0.0f;
	float m_val4 = 0.0f;

	void SetConstBufferVar(_Out_ float* var1, _Out_ float* var2, _Out_ float* var3, _Out_ float* var4) const override;

public:
	Player(const std::vector<std::wstring>& shapes) : Object(shapes, VertexShaders::TripleInput) {}
	~Player() override = default;

	void Update(float deltaTime) override;

	void MoveToTarget(const DirectX::XMVECTOR& target, const DirectX::XMVECTOR& targetRotation);
};