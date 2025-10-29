#include "TestScene.h"

using namespace std;
using namespace DirectX;

TestScene::TestScene()
{
	m_backgroundColor = XMFLOAT4{ m_timeColors[m_timeOfDay].x, m_timeColors[m_timeOfDay].y, m_timeColors[m_timeOfDay].z, 1.0f };
	m_ambientFog.x = m_timeColors[m_timeOfDay].x;
	m_ambientFog.y = m_timeColors[m_timeOfDay].y;
	m_ambientFog.z = m_timeColors[m_timeOfDay].z;

	m_camera->SetPosition(XMVECTOR{ 0.0f, 10.0f, -20.0f, 1.0f });
	m_camera->LookAt(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });
	m_camera->AddComponent<Camera>();

	m_windmill->SetPosition(XMVECTOR{ 5.0f, 0.0f, 5.0f, 1.0f });
	m_windmill->SetScale(XMFLOAT3{ 1.5f, 1.5f, 1.5f });

	unique_ptr<Object> plane = make_unique<Object>();
	plane->AddComponent<Shape>(L"PlaneX10", L"VertexShader", L"PixelShader", vector<wstring>{ L"Plain", L"PlainNormal" });
	plane->SetScale(XMFLOAT3{ 100.0f, 1.0f, 100.0f });
	m_objects.emplace_back(move(plane));

	m_lightObj = make_unique<Object>();
	m_lightObj->SetPosition(XMVECTOR{ 5.0f, 5.0f, 0.0f, 1.0f });
	m_lightObj->AddComponent<DirectionalLight>(m_morningColor, 0.75f, XMVECTOR{ 1.0f, -1.0f, 1.0f, 0.0f });
	m_lightObj->AddComponent<PointLight>(XMFLOAT3{ 0.0f, 1.0f, 1.0f }, 5.0f, 60.0f);
	m_lightObj->AddComponent<Shape>(L"Eye", L"VertexShader", L"PixelShader", vector<wstring>{ L"Eye", L"NoNormal" });

	const int treeCount = 1000;
	for (int i = 0; i < treeCount; ++i)
	{
		float x = static_cast<float>((rand() % 2000) - 1000) / 10.0f;
		float z = static_cast<float>((rand() % 2000) - 1000) / 10.0f;
		unique_ptr<Object> tree = make_unique<Tree>();
		tree->SetPosition(XMVECTOR{ x, 0.0f, z, 1.0f });
		m_objects.emplace_back(move(tree));
	}

	m_player = make_unique<Player>();

	m_player->AddChild(m_camera.get());
	m_player->SetPosition(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });

	m_torch = make_unique<Object>();
	m_torch->AddComponent<Shape>(L"Eye", L"VertexShader", L"PixelShader", vector<wstring>{ L"Eye", L"NoNormal" });
	m_torch->SetScale(XMFLOAT3{ 0.75f, 0.75f, 0.75f });
	m_torch->SetPosition(XMVECTOR{ 0.0f, 5.0f, 0.0f, 1.0f });
	m_torch->LookAt(XMVECTOR{ 0.0f, 0.0f, 5.0f, 1.0f });
	m_torch->AddComponent<PointLight>(XMFLOAT3{ 1.0f, 0.5f, 0.0f }, 5.0f, 30.0f);
	m_player->AddChild(m_torch.get());
}

void TestScene::Update(float deltaTime)
{
	m_player->Update(deltaTime);
	m_windmill->Update(deltaTime);
	m_lightObj->LookAt(m_player->GetWorldPosition());

	if (GetAsyncKeyState(VK_F6) & 0x0001)
	{
		m_timeOfDay = static_cast<TimeOfDay>((m_timeOfDay + 1) % TimeOfDayCount);
		m_backgroundColor = XMFLOAT4{ m_timeColors[m_timeOfDay].x, m_timeColors[m_timeOfDay].y, m_timeColors[m_timeOfDay].z, 1.0f };
		m_ambientFog.x = m_timeColors[m_timeOfDay].x;
		m_ambientFog.y = m_timeColors[m_timeOfDay].y;
		m_ambientFog.z = m_timeColors[m_timeOfDay].z;
		m_ambientLight = XMFLOAT4{ m_timeColors[m_timeOfDay].x * 0.25f, m_timeColors[m_timeOfDay].y * 0.25f, m_timeColors[m_timeOfDay].z * 0.25f, 1.0f };
		auto directionalLight = m_lightObj->GetComponent<DirectionalLight>();
		directionalLight->SetColor(m_timeColors[m_timeOfDay]);
	}

	if (GetAsyncKeyState('W') & 0x8000) m_torch->Rotate(XMVECTOR{ -1.0f * deltaTime, 0.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('S') & 0x8000) m_torch->Rotate(XMVECTOR{ 1.0f * deltaTime, 0.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('A') & 0x8000) m_torch->Rotate(XMVECTOR{ 0.0f, -1.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState('D') & 0x8000) m_torch->Rotate(XMVECTOR{ 0.0f, 1.0f * deltaTime, 0.0f, 0.0f });

	Scene::Update(deltaTime);
}