#include "Player.h"

using namespace DirectX;

Player::Player(Shapes shape) : Object(shape)
{
}

void Player::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	XMVECTOR moveDelta = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	if (GetAsyncKeyState(VK_UP) & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));

	XMVECTOR normMove = XMVector3Normalize(moveDelta);
	normMove *= m_moveSpeed * deltaTime;
	MovePosition(normMove);
}