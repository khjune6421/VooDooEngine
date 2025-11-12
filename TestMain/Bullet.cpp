#include "pch.h"
#include "Bullet.h"

using namespace std;
using namespace DirectX;

Bullet::Bullet(Scene* scene) : Object(scene)
{
	AddComponent<Shape>(L"WindmillWings", L"VertexShader", L"PixelShader", L"Wood");
	AddComponent<Collider>(1.0f, vector<UINT>{ 1 });
}

void Bullet::Update(float deltaTime)
{
	MoveDirection(Directions::Forward, 20.0f * deltaTime);
	Rotate(XMVECTOR{ 0.0f, 0.0f, 5.0f * deltaTime, 0.0f });

	m_lifeTime -= deltaTime;
}