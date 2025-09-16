#include "TestScene.h"

using namespace std;

TestScene::TestScene()
{
	//m_objects.emplace_back(make_unique<Object>(Shapes::Triangle));
	//m_objects.emplace_back(make_unique<Object>(Shapes::Square));
	m_objects.emplace_back(make_unique<Object>(Shapes::Tetrahedron));
	//m_objects.emplace_back(make_unique<Object>(Shapes::Cube));
}

void TestScene::Update(float deltaTime)
{
	for (const auto & object : m_objects)
	{
		object->GetTransform().rotation.y += deltaTime;
	}
}