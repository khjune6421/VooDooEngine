#include "Player.h"

using namespace DirectX;

Player::Player(Shapes shape) : Object(shape)
{
}

void Player::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	XMFLOAT3 moveDelta(0.0f, 0.0f, 0.0f);

	if (GetAsyncKeyState(VK_UP) & 0x8000) moveDelta.z += m_moveSpeed * deltaTime;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) moveDelta.z -= m_moveSpeed * deltaTime;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) moveDelta.x -= m_moveSpeed * deltaTime;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) moveDelta.x += m_moveSpeed * deltaTime;

	// Use this later
	XMVECTOR norm = XMVector3Normalize(XMVectorSet(moveDelta.x, 0.0f, moveDelta.z, 0.0f));

	MovePosition(moveDelta);
}