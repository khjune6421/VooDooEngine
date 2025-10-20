#include "TestScene.h"

using namespace std;
using namespace DirectX;

TestScene::TestScene(wstring dataFile)
{
	m_camera->SetPosition(XMVECTOR{ 0.0f, 10.0f, -20.0f, 1.0f });
	m_camera->LookAt(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });
	m_camera->AddComponent<Camera>();

	m_player->AddChild(m_camera.get());
	m_player->SetPosition(XMVECTOR{ 0.0f, 1.0f, 0.0f, 1.0f });
	m_player->AddComponent<PointLight>(XMFLOAT3{ 1.0f, 1.0f, 1.0f }, 3.0f);
	m_windmill->SetPosition(XMVECTOR{ 5.0f, 0.0f, 5.0f, 1.0f });
	m_windmill->SetScale(XMFLOAT3{ 1.5f, 1.5f, 1.5f });

	unique_ptr<Object> plane = make_unique<Object>();
	plane->AddComponent<Shape>(L"Plane", L"VertexShader", L"PixelShader", L"Brick");
	plane->SetScale(XMFLOAT3{ 100.0f, 1.0f, 100.0f });
	m_objects.emplace_back(move(plane));

	unique_ptr<Object> lightObj = make_unique<Object>();
	lightObj->SetPosition(XMVECTOR{ 5.0f, 5.0f, 0.0f, 1.0f });
	//lightObj->AddComponent<AmbientLight>(XMFLOAT3{ 1.0f, 1.0f, 1.0f }, 0.1f);
	//lightObj->AddComponent<DirectionalLight>(XMFLOAT3{ 1.0f, 1.0f, 1.0f }, 0.1f, XMVECTOR{ -1.0f, -1.0f, 1.0f, 0.0f });
	lightObj->AddComponent<Shape>(L"Sphere");
	m_objects.emplace_back(move(lightObj));

	ObjectPositionParser parser;
	if (parser.LoadPositions(dataFile))
	{
		for (const auto& pos : parser.GetPositions())
		{
			unique_ptr<Object> tree = std::make_unique<Windmill>();
			tree->SetPosition(XMVECTOR{ pos.x, pos.y, pos.z, 1.0f });
			tree->SetRotation(XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
			m_objects.emplace_back(move(tree));
		}
	}
	else
	{
		MessageBoxW(nullptr, L"Failed to load object positions", L"Error", MB_OK);
	}
}

void TestScene::Update(float deltaTime)
{
	for (const auto& object : m_objects) object->Update(deltaTime);
	m_player->Update(deltaTime);
	m_windmill->Update(deltaTime);
}