#include "TestScene.h"

#include "ObjectPositionParser.h"

using namespace std;
using namespace DirectX;

TestScene::TestScene(string dataFile)
{
	m_player = make_unique<Player>(Shapes::Cube);
	m_player->SetPosition(XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_windmill = make_unique<WindMill>(Shapes::Tetrahedron);
	m_windmill->SetPosition(XMFLOAT3(10.0f, 0.0f, 10.0f));

	unique_ptr<Object> plane = make_unique<Object>(Shapes::Plane);
	plane->SetScale(XMFLOAT3(50.0f, 1.0f, 50.0f));
	m_objects.emplace_back(move(plane));

	ObjectPositionParser parser;
	if (parser.LoadPositions(dataFile))
	{
		for (const auto& pos : parser.GetPositions())
		{
			unique_ptr<Object> tree = make_unique<Object>(Shapes::Tree);
			tree->SetPosition(XMFLOAT3(pos.x, pos.y, pos.z));
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

	if (GetAsyncKeyState(VK_F5) & 0x0001)
	{
		if (m_player->m_windmillWing)
		{
			m_windmill->m_windmillWing = move(m_player->m_windmillWing);
			m_windmill->AddChild(m_windmill->m_windmillWing.get());
			m_windmill->m_windmillWing->m_rotationAngle = false;

			m_player->m_childrens.clear();
		}
		else if (m_windmill->m_windmillWing)
		{
			m_player->m_windmillWing = move(m_windmill->m_windmillWing);
			m_player->AddChild(m_player->m_windmillWing.get());
			m_player->m_windmillWing->m_rotationAngle = true;

			m_windmill->m_childrens.clear();
		}
	}

	g_camera.LookAt(m_player->GetPosition());
}