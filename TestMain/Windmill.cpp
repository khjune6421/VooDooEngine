#include "Windmill.h"

using namespace DirectX;
using namespace std;

Windmill::Windmill(Scene* scene) : Object(scene)
{
	AddComponent<Shape>(L"Windmill", L"VertexShader", L"PixelShader", vector<wstring>{ L"Building", L"BuildingNormal" });

	m_Wing = make_unique<Object>(scene);
	m_Wing->AddComponent<Shape>(L"WindmillWings", L"VertexShader", L"PixelShader", vector<wstring>{ L"Wood", L"NoNormal" });
	m_Wing->SetPosition(XMVECTOR{ 0.0f, 1.0f, -0.75f, 1.0f });
	m_Wing->SetScale(XMFLOAT3{ 0.75f, 0.75f, 0.75f });
	AddChild(m_Wing.get());
}

void Windmill::Update(float deltaTime)
{
	m_Wing->Rotate(XMVECTOR{ 0.0f, 0.0f, deltaTime, 0.0f });
}