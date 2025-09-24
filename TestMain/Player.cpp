#include "Player.h"

using namespace DirectX;

Player::Player(Shapes shape) : Object(shape)
{
	m_camera->SetPosition(XMVECTOR{ 0.0f, 10.0f, -20.0f, 1.0f });
	m_camera->LookAt(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });
	AddChild(m_camera.get());
}

void Player::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	if (GetAsyncKeyState(VK_UP) & 0x8000) MoveDirection(Directions::Forward, m_moveSpeed * deltaTime);
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) MoveDirection(Directions::Backward, m_moveSpeed / 2.0f * deltaTime);

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate(XMVECTOR{ 0.0f, -2.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate(XMVECTOR{ 0.0f, 2.0f * deltaTime, 0.0f, 0.0f });
}