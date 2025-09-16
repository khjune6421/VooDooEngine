#include "TestScene.h"

using namespace std;

TestScene::TestScene()
{
	m_objects.emplace_back(make_unique<Object>(Shapes::Triangle));
	m_objects.emplace_back(make_unique<Object>(Shapes::Cube));
	m_objects.emplace_back(make_unique<Object>(Shapes::Tetrahedron));
	m_objects.emplace_back(make_unique<Object>(Shapes::Square));
}

void TestScene::Update(float deltaTime)
{
	for (size_t i = 0; i < m_objects.size(); ++i)
	{
		m_objects[i]->GetTransform().rotation.y += deltaTime;
		//m_objects[i]->GetTransform().position.x = static_cast<float>(i * 3 - 4);
	}
}