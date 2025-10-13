#include "Player.h"

using namespace std;
using namespace DirectX;

void Player::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate(XMVECTOR{ 0.0f, -2.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate(XMVECTOR{ 0.0f, 2.0f * deltaTime, 0.0f, 0.0f });

	if (m_isMovingToTarget)
	{
		ATime += deltaTime * 5.0f;
		m_interpolationFactor += deltaTime * 3.0f;

		if (m_interpolationFactor > 1.0f)
		{
			m_currentShapeIndex = m_nextShapeIndex;
			if (m_nextShapeIndex == 1) m_nextShapeIndex = 2;
			else if (m_nextShapeIndex == 2) m_nextShapeIndex = 1;

			m_interpolationFactor = 0.0f;
		}

		m_moveToTargetElapsed += deltaTime;
		float t = m_moveToTargetElapsed / m_moveToTargetTime;

		if (t >= 1.0f)
		{
			t = 1.0f;
			m_isMovingToTarget = false;

			m_nextShapeIndex = 0;
			m_interpolationFactor = 0.0f;
		}
		LerpPosition(m_startPosition, m_targetPosition, t);
		float rot = clamp(t * 5.0f, 0.0f, 1.0f);
		LerpRotation(m_startRotation, m_targetRotation, rot);
	}
	else m_interpolationFactor = clamp(m_interpolationFactor + deltaTime * 3.0f, 0.0f, 1.0f);
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

	m_currentShapeIndex = 0;
	m_nextShapeIndex = 1;
	m_interpolationFactor = 0.0f;
}