#include "TestScene.h"
#include "ObjectPositionParser.h"
#include "Tree.h"

using namespace std;

TestScene::TestScene(string dataFile)
{
	unique_ptr<Object> plane = make_unique<Object>(Shapes::Plane);
	plane->GetTransform().scale = { 100.0f, 1.0f, 100.0f };
	m_objects.emplace_back(move(plane));

	ObjectPositionParser parser;
	if (parser.LoadPositions(dataFile))
	{
		for (const auto& pos : parser.GetPositions())
		{
			unique_ptr<Tree> tree = make_unique<Tree>();
			tree->GetTransform().position = { pos.x, pos.y, pos.z };
			tree->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
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
	//for (const auto & object : m_objects)
	//{
	//	object->GetTransform().rotation.y += deltaTime;
	//}
}