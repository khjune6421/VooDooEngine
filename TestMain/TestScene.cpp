#include "cPch.h"
#include "TestScene.h"

#include "Object.h"
#include "Camera.h"

using namespace std;
using namespace DirectX;

TestScene::TestScene()
{
	m_backgroundColor = XMFLOAT4{ m_timeColors[m_timeOfDay].x, m_timeColors[m_timeOfDay].y, m_timeColors[m_timeOfDay].z, 1.0f };
	m_ambientFog.x = m_timeColors[m_timeOfDay].x;
	m_ambientFog.y = m_timeColors[m_timeOfDay].y;
	m_ambientFog.z = m_timeColors[m_timeOfDay].z;
	m_ambientLight = XMFLOAT4{ m_timeColors[m_timeOfDay].x * 0.25f, m_timeColors[m_timeOfDay].y * 0.25f, m_timeColors[m_timeOfDay].z * 0.25f, 1.0f };

	m_directionalLight.color = XMFLOAT4{ m_timeColors[m_timeOfDay].x * 0.75f, m_timeColors[m_timeOfDay].y * 0.75f, m_timeColors[m_timeOfDay].z * 0.75f, 0.0f };
	m_directionalLight.direction = XMVector3Normalize(XMVECTOR{ 1.0f, -1.0f, 1.0f, 0.0f });

	m_camera->SetPosition(XMVECTOR{ 0.0f, 10.0f, -20.0f, 1.0f });
	m_camera->LookAt(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });
	m_camera->AddComponent<Camera>(1280, 720);

	m_windmill->SetPosition(XMVECTOR{ 2.5f, 0.1f, 2.5f, 1.0f });
	m_windmill->SetScale(XMFLOAT3{ 1.5f, 1.5f, 1.5f });

	unique_ptr<Object> plane = make_unique<Object>(this);
	plane->AddComponent<Shape>(L"PlaneX10", L"VertexShader", L"PixelShader", L"Plain");
	plane->SetScale(XMFLOAT3{ 100.0f, 1.0f, 100.0f });
	m_objects.emplace_back(move(plane));

	m_lightObj = make_unique<Object>(this);
	m_lightObj->SetPosition(XMVECTOR{ 2.5f, 1.5f, 2.5f, 1.0f });
	m_lightObj->AddComponent<PointLight>(XMFLOAT3{ 1.0f, 0.75f, 0.75f }, 5.0f, 0.0f, 20.0f, 1.0f, 0.25f, 0.25f);

	constexpr int treeCount = 1000;
	for (int i = 0; i < treeCount; ++i)
	{
		float x = static_cast<float>((rand() % 2000) - 1000) / 10.0f;
		float z = static_cast<float>((rand() % 2000) - 1000) / 10.0f;
		unique_ptr<Object> tree = make_unique<Tree>(this);
		tree->SetPosition(XMVECTOR{ x, 0.0f, z, 1.0f });
		m_objects.emplace_back(move(tree));
	}

	m_player = make_unique<Player>(this);
	m_player->AddComponent<Shape>(L"Player", L"VertexShader", L"PixelShader", L"Lilypads");

	m_player->AddChild(m_camera.get());
	m_player->SetPosition(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });

	m_torch = make_unique<Object>(this);
	m_torch->AddComponent<Shape>(L"Eye", L"VertexShader", L"PixelShader", L"Eye");
	m_torch->SetScale(XMFLOAT3{ 0.25f, 0.25f, 0.25f });
	m_torch->SetPosition(XMVECTOR{ 0.0f, 2.5f, 0.0f, 1.0f });
	//m_torch->AddComponent<PointLight>(XMFLOAT3{ 1.0f, 0.75f, 0.5f }, 2.5f, 0.0f, 50.0f, 1.0f, 0.5f, 0.5f);
	m_torch->AddComponent<PointLight>(XMFLOAT3{ 1.0f, 0.75f, 0.5f }, 1.0f);
	m_player->AddChild(m_torch.get());
}

void TestScene::Update(float deltaTime)
{
	m_player->Update(deltaTime);
	m_windmill->Update(deltaTime);

	if (GetAsyncKeyState(VK_F6) & 0x0001)
	{
		m_timeOfDay = static_cast<TimeOfDay>((m_timeOfDay + 1) % TimeOfDayCount);
		m_backgroundColor = XMFLOAT4{ m_timeColors[m_timeOfDay].x, m_timeColors[m_timeOfDay].y, m_timeColors[m_timeOfDay].z, 1.0f };
		m_ambientFog.x = m_timeColors[m_timeOfDay].x;
		m_ambientFog.y = m_timeColors[m_timeOfDay].y;
		m_ambientFog.z = m_timeColors[m_timeOfDay].z;
		m_ambientLight = XMFLOAT4{ m_timeColors[m_timeOfDay].x * 0.25f, m_timeColors[m_timeOfDay].y * 0.25f, m_timeColors[m_timeOfDay].z * 0.25f, 1.0f };
		m_directionalLight.color = XMFLOAT4{ m_timeColors[m_timeOfDay].x * 0.75f, m_timeColors[m_timeOfDay].y * 0.75f, m_timeColors[m_timeOfDay].z * 0.75f, 0.0f };
	}

	Scene::Update(deltaTime);
}