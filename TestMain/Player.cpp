#include "Player.h"

using namespace DirectX;
using namespace std;

Player::Player()
{
	AddComponent<Shape>(L"Player", L"VertexShader", L"PixelShader", vector<wstring>{ L"Monkey", L"NoNormal" });
}

void Player::Update(float deltaTime)
{
	if (GetAsyncKeyState(VK_UP) & 0x8000) MoveDirection(Directions::Forward, m_moveSpeed * deltaTime);
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) MoveDirection(Directions::Backward, m_moveSpeed / 2.0f * deltaTime);

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate(XMVECTOR{ 0.0f, -2.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate(XMVECTOR{ 0.0f, 2.0f * deltaTime, 0.0f, 0.0f });
}