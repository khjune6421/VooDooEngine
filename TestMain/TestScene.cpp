#include "TestScene.h"

#include "ObjectPositionParser.h"

using namespace std;
using namespace DirectX;

TestScene::TestScene(string dataFile)
{
	m_player = make_unique<Player>(Shapes::Triangle);
	m_player->SetPosition(XMVECTOR{ 0.0f, 2.0f, 0.0f, 1.0f });
	m_player->SetScale(XMVECTOR{ 2.0f, 2.0f, 2.0f, 0.0f });

	m_windmill = make_unique<WindMill>(Shapes::Tetrahedron);
	m_windmill->SetPosition(XMVECTOR{ 10.0f, 2.0f, 10.0f, 1.0f });
	m_windmill->SetScale(XMVECTOR{ 2.0f, 2.0f, 2.0f, 0.0f });

	unique_ptr<Object> plane = make_unique<Object>(Shapes::Plane);
	plane->SetScale(XMVECTOR{ 50.0f, 1.0f, 50.0f, 0.0f });
	m_objects.emplace_back(move(plane));

	ObjectPositionParser parser;
	if (parser.LoadPositions(dataFile))
	{
		int index = 0;
		for (const auto& pos : parser.GetPositions())
		{
			index++;
			unique_ptr<Object> tree;
			if (index % 3) tree = make_unique<Object>(Shapes::Tree);
			else tree = make_unique<Object>(Shapes::Tree, VertexShaders::Default, PixelShaders::Greyscale);
			tree->SetPosition(XMVECTOR{ pos.x, pos.y, pos.z, 1.0f });
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
	m_player->Update(deltaTime);
	m_windmill->Update(deltaTime);
	for (const auto& object : m_objects) object->Update(deltaTime);

	if (GetAsyncKeyState('C') & 0x0001)
	{
		if (m_windmill->m_windmillWing)
		{
			m_windmill->m_childrens.clear();

			m_player->m_windmillWing = move(m_windmill->m_windmillWing);
			m_player->AddChild(m_player->m_windmillWing.get());
			m_player->m_windmillWing->SetPosition(XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
			m_player->m_windmillWing->SetScale(XMVECTOR{ 1.0f, 1.0f, 1.0f, 0.0f });
			m_player->m_windmillWing->SetRotation(XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
		}
	}
	if (GetAsyncKeyState('V') & 0x0001)
	{
		if (m_player->m_windmillWing)
		{
			m_projectileWing = move(m_player->m_windmillWing);

			m_player->m_childrens.clear();
			m_projectileWing->SetPosition(m_player->GetPosition());
			m_projectileWing->SetRotation(m_player->GetRotation());
			m_projectileWing->SetScale(m_player->GetScale());
			m_projectileWing->m_parent = nullptr;
		}
	}
	if (m_projectileWing)
	{
		static float lifeTime = 1.0f;
		lifeTime -= deltaTime;

		m_projectileWing->MoveDirection(Directions::Forward, 20.0f * deltaTime);
		m_projectileWing->Update(deltaTime);

		if (lifeTime <= 0.0f)
		{
			m_projectileWing->SetPosition(m_windmill->GetPosition());
			m_projectileWing->SetRotation(XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
			m_projectileWing->SetScale(m_windmill->GetScale());
			m_windmill->m_windmillWing = move(m_projectileWing);
			m_windmill->AddChild(m_projectileWing.get());
			lifeTime = 1.0f;
		}
	}

	g_camera.LookAt(m_player->GetPosition());
}