#include "TestScene.h"

using namespace std;
using namespace DirectX;

TestScene::TestScene()
{
	m_backgroundColor = XMFLOAT4{ m_morningColor.x, m_morningColor.y, m_morningColor.z, 1.0f };

	m_camera->SetPosition(XMVECTOR{ 0.0f, 10.0f, -20.0f, 1.0f });
	m_camera->LookAt(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });
	m_camera->AddComponent<Camera>();

	m_player->AddChild(m_camera.get());
	m_player->SetPosition(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });

	m_torch = make_unique<Object>();
	m_torch->AddComponent<Shape>(L"Sphere");
	m_torch->SetScale(XMFLOAT3{ 0.1f, 0.1f, 0.1f });
	m_torch->SetPosition(XMVECTOR{ 1.0f, 2.5f, -1.0f, 1.0f });
	m_torch->LookAt(XMVECTOR{ 1.0f, 2.0f, 1.0f, 1.0f });
	m_torch->AddComponent<PointLight>(XMFLOAT3{ 1.0f, 0.5f, 0.0f }, 5.0f, 30.0f);
	m_player->AddChild(m_torch.get());

	m_windmill->SetPosition(XMVECTOR{ 5.0f, 0.0f, 5.0f, 1.0f });
	m_windmill->SetScale(XMFLOAT3{ 1.5f, 1.5f, 1.5f });

	unique_ptr<Object> plane = make_unique<Object>();
	plane->AddComponent<Shape>(L"PlaneX10", L"VertexShader", L"PixelShader", L"Brick");
	plane->SetScale(XMFLOAT3{ 100.0f, 1.0f, 100.0f });
	m_objects.emplace_back(move(plane));

	m_lightObj = make_unique<Object>();
	m_lightObj->SetPosition(XMVECTOR{ 5.0f, 5.0f, 0.0f, 1.0f });
	m_lightObj->AddComponent<AmbientLight>(m_morningColor, 0.25f);
	m_lightObj->AddComponent<DirectionalLight>(m_morningColor, 0.75f, XMVECTOR{ 1.0f, -1.0f, 1.0f, 0.0f });
	m_lightObj->AddComponent<PointLight>(XMFLOAT3{ 0.0f, 1.0f, 1.0f }, 5.0f, 60.0f);
	m_lightObj->AddComponent<Shape>(L"Sphere");

	const int treeCount = 1000;
	for (int i = 0; i < treeCount; ++i)
	{
		float x = static_cast<float>((rand() % 2000) - 1000) / 10.0f;
		float z = static_cast<float>((rand() % 2000) - 1000) / 10.0f;
		unique_ptr<Object> tree = make_unique<Tree>();
		tree->AddComponent<Shape>(L"Tree", L"VertexShader", L"PixelShader", L"Lilypads");
		tree->SetPosition(XMVECTOR{ x, 0.0f, z, 1.0f });
		tree->SetRotation(XMVECTOR{ 0.0f, XM_PI, 0.0f, 0.0f });
		m_objects.emplace_back(move(tree));
	}
}

void TestScene::Update(float deltaTime)
{
	for (const auto& object : m_objects) object->Update(deltaTime);
	m_player->Update(deltaTime);
	m_windmill->Update(deltaTime);
	m_lightObj->LookAt(m_player->GetWorldPosition());

	if (GetAsyncKeyState(VK_F6) & 0x0001)
	{
		m_backgroundColor = XMFLOAT4{ m_morningColor.x, m_morningColor.y, m_morningColor.z, 1.0f };
		auto ambientLight = m_lightObj->GetComponent<AmbientLight>();
		ambientLight->SetColor(m_morningColor);
		auto directionalLight = m_lightObj->GetComponent<DirectionalLight>();
		directionalLight->SetColor(m_morningColor);
	}
	if (GetAsyncKeyState(VK_F7) & 0x0001)
	{
		m_backgroundColor = XMFLOAT4{ m_noonColor.x, m_noonColor.y, m_noonColor.z, 1.0f };
		auto ambientLight = m_lightObj->GetComponent<AmbientLight>();
		ambientLight->SetColor(m_noonColor);
		auto directionalLight = m_lightObj->GetComponent<DirectionalLight>();
		directionalLight->SetColor(m_noonColor);
	}
	if (GetAsyncKeyState(VK_F8) & 0x0001)
	{
		m_backgroundColor = XMFLOAT4{ m_nightColor.x, m_nightColor.y, m_nightColor.z, 1.0f };
		auto ambientLight = m_lightObj->GetComponent<AmbientLight>();
		ambientLight->SetColor(m_nightColor);
		auto directionalLight = m_lightObj->GetComponent<DirectionalLight>();
		directionalLight->SetColor(m_nightColor);
	}

	if (GetAsyncKeyState('W') & 0x8000) m_torch->Rotate(XMVECTOR{ -1.0f * deltaTime, 0.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('S') & 0x8000) m_torch->Rotate(XMVECTOR{ 1.0f * deltaTime, 0.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('A') & 0x8000) m_torch->Rotate(XMVECTOR{ 0.0f, -1.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState('D') & 0x8000) m_torch->Rotate(XMVECTOR{ 0.0f, 1.0f * deltaTime, 0.0f, 0.0f });

	if (GetAsyncKeyState('Z') & 0x8000) m_windmill->Scale(XMFLOAT3{ 1.0f, 1.0f + deltaTime, 1.0f });
	if (GetAsyncKeyState('X') & 0x8000) m_windmill->Scale(XMFLOAT3{ 1.0f, 1.0f - deltaTime, 1.0f });

	if (GetAsyncKeyState('C') & 0x8000) m_player->Scale(XMFLOAT3{ 1.0f, 1.0f + deltaTime, 1.0f });
	if (GetAsyncKeyState('V') & 0x8000) m_player->Scale(XMFLOAT3{ 1.0f, 1.0f - deltaTime, 1.0f });
}