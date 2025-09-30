#include "Player.h"

using namespace DirectX;

void Player::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	//if (GetAsyncKeyState(VK_UP) & 0x8000) MoveDirection(Directions::Forward, m_moveSpeed * deltaTime);
	//if (GetAsyncKeyState(VK_DOWN) & 0x8000) MoveDirection(Directions::Backward, m_moveSpeed / 2.0f * deltaTime);

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate(XMVECTOR{ 0.0f, -2.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate(XMVECTOR{ 0.0f, 2.0f * deltaTime, 0.0f, 0.0f });

	if (m_isMovingToTarget)
	{
		m_moveToTargetElapsed += deltaTime;
		float t = m_moveToTargetElapsed / m_moveToTargetTime;

		if (t >= 1.0f)
		{
			t = 1.0f;
			m_isMovingToTarget = false;
		}
		LerpPosition(m_startPosition, m_targetPosition, t);
	}
}

void Player::MoveToTarget(const XMVECTOR& target)
{
	m_targetPosition = target;
	m_isMovingToTarget = true;

	m_startPosition = GetWorldPosition();
	XMVECTOR toTarget = XMVectorSubtract(m_targetPosition, m_startPosition);
	float distance = XMVectorGetX(XMVector3Length(toTarget));

	m_moveToTargetTime = distance / m_moveSpeed;
	m_moveToTargetElapsed = 0.0f;
}