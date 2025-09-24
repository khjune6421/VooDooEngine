#include "TestScene.h"

using namespace std;
using namespace DirectX;

TestScene::TestScene(wstring dataFile)
{
	m_player = make_unique<Player>(Shapes::Triangle);
	m_player->SetPosition(XMVECTOR{ 0.0f, 2.0f, 0.0f, 1.0f });
	m_player->SetScale(XMFLOAT3{ 2.0f, 2.0f, 2.0f });

	m_windmill = make_unique<WindMill>(Shapes::Tetrahedron);
	m_windmill->SetPosition(XMVECTOR{ 10.0f, 2.0f, 10.0f, 1.0f });
	m_windmill->SetScale(XMFLOAT3{ 2.0f, 2.0f, 2.0f });

	unique_ptr<Object> plane = make_unique<Object>(Shapes::Plane);
	plane->SetScale(XMFLOAT3{ 100.0f, 1.0f, 100.0f });
	m_objects.emplace_back(move(plane));

	ObjectPositionParser parser;
	if (parser.LoadPositions(dataFile))
	{
		for (const auto& pos : parser.GetPositions())
		{
			unique_ptr<Object> tree = make_unique<Tree>();
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
			m_windmill->RemoveChild(m_windmill->m_windmillWing.get());

			m_player->m_windmillWing = move(m_windmill->m_windmillWing);
			m_player->AddChild(m_player->m_windmillWing.get());
			m_player->m_windmillWing->SetPosition(XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
			m_player->m_windmillWing->SetScale(XMFLOAT3{ 1.0f, 1.0f, 1.0f });
			m_player->m_windmillWing->SetRotation(XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
		}
	}
	if (GetAsyncKeyState('V') & 0x0001)
	{
		if (m_player->m_windmillWing)
		{
			m_projectileWing = move(m_player->m_windmillWing);
			m_player->RemoveChild(m_projectileWing.get());

			m_projectileWing->SetPosition(m_player->GetPosition());
			m_projectileWing->SetRotation(m_player->GetRotation());
			m_projectileWing->SetScale(m_player->GetScale());

			g_collidibleObjects.emplace_back(m_projectileWing.get());
		}
	}
	if (m_projectileWing)
	{
		static float lifeTime = 3.0f;
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
			lifeTime = 3.0f;

			g_collidibleObjects.erase
			(
				remove
				(
					g_collidibleObjects.begin(),
					g_collidibleObjects.end(),
					m_windmill->m_windmillWing.get()
				),
				g_collidibleObjects.end()
			);
		}
	}
}