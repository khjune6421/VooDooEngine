#include "cPch.h"
#include "Player.h"

#include "Component.h"

using namespace DirectX;
using namespace std;

void Player::Update(float deltaTime)
{
	if (GetAsyncKeyState(VK_UP) & 0x8000) MoveDirection(Directions::Forward, m_moveSpeed * deltaTime);
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) MoveDirection(Directions::Backward, m_moveSpeed / 2.0f * deltaTime);

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate(XMVECTOR{ 0.0f, -2.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate(XMVECTOR{ 0.0f, 2.0f * deltaTime, 0.0f, 0.0f });

	if (GetAsyncKeyState(VK_SPACE) & 0x0001)
	{
		unique_ptr<Bullet> bullet = make_unique<Bullet>(m_scene);
		bullet->SetPosition(GetWorldPosition());
		bullet->MovePosition(XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f });
		bullet->SetRotation(m_rotation);
		m_bullets.push_back(move(bullet));
	}

	for (auto& bullet : m_bullets) bullet->Update(deltaTime);
	if (!m_bullets.empty() && m_bullets.front()->IsExpired()) m_bullets.pop_front();
}