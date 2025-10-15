#include "Windmill.h"

using namespace DirectX;

Windmill::Windmill()
{
	AddComponent<Shape>(L"Pyramid");

	m_Wing->AddComponent<Shape>(L"WindmillWings");
	m_Wing->SetPosition(XMVECTOR{ 0.0f, 1.0f, -0.75f, 1.0f });
	m_Wing->SetScale(XMFLOAT3{ 0.75f, 0.75f, 0.75f });
	AddChild(m_Wing.get());
}

void Windmill::Update(float deltaTime)
{
	m_Wing->Rotate(XMVECTOR{ 0.0f, 0.0f, deltaTime, 0.0f });
}