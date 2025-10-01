#include "Player.h"

using namespace std;
using namespace DirectX;

void Player::SetConstBufferVar(_Out_ float* var1, _Out_ float* var2, _Out_ float* var3, _Out_ float* var4) const
{
	*var1 = m_val1;
	*var2 = m_val2;
	*var3 = m_val3;
	*var4 = m_val4;
}

void Player::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate(XMVECTOR{ 0.0f, -2.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate(XMVECTOR{ 0.0f, 2.0f * deltaTime, 0.0f, 0.0f });

	if (m_isMovingToTarget)
	{

		ATime += deltaTime * 5.0f;
		m_val1 = sin(ATime) / 2.0f + 0.5f;

		m_moveToTargetElapsed += deltaTime;
		float t = m_moveToTargetElapsed / m_moveToTargetTime;

		if (t >= 1.0f)
		{
			t = 1.0f;
			m_isMovingToTarget = false;
		}
		LerpPosition(m_startPosition, m_targetPosition, t);
		m_val2 = clamp(m_val2 + deltaTime * 5.0f, 0.0f, 1.0f);
		LerpRotation(m_startRotation, m_targetRotation, m_val2);
	}
	else
	{
		if (m_val2 > 0.0f) m_val2 -= deltaTime * 5.0f;
	}
}

void Player::MoveToTarget(const XMVECTOR& target, const XMVECTOR& targetRotation)
{
	m_targetPosition = target;
	m_isMovingToTarget = true;

	m_startPosition = GetWorldPosition();
	XMVECTOR toTarget = XMVectorSubtract(m_targetPosition, m_startPosition);
	float distance = XMVectorGetX(XMVector3Length(toTarget));

	m_startRotation = m_rotation;
	m_targetRotation = targetRotation;

	m_moveToTargetTime = distance / m_moveSpeed;
	m_moveToTargetElapsed = 0.0f;

	ATime = 0.0f;
	m_val1 = 0.0f;
	m_val2 = 0.0f;
}