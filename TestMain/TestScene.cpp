#include "TestScene.h"

#include "ObjectPositionParser.h"

using namespace std;

TestScene::TestScene(string dataFile)
{
	m_player = make_unique<Player>(Shapes::Cube);
	m_player->SetPosition({ 0.0f, 1.0f, 0.0f });

	m_windmill = make_unique<WindMill>(Shapes::Tetrahedron);
	m_windmill->SetPosition({ 10.0f, 0.0f, 10.0f });

	unique_ptr<Object> plane = make_unique<Object>(Shapes::Plane);
	plane->SetScale({ 100.0f, 1.0f, 100.0f });
	m_objects.emplace_back(move(plane));

	ObjectPositionParser parser;
	if (parser.LoadPositions(dataFile))
	{
		for (const auto& pos : parser.GetPositions())
		{
			unique_ptr<Object> tree = make_unique<Object>(Shapes::Tree);
			tree->SetPosition({ pos.x, pos.y, pos.z });
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
}