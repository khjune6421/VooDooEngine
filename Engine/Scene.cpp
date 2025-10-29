#include "Scene.h"

void Scene::Update(float deltaTime)
{
	for (const auto& object : m_objects) object->Update(deltaTime);
	// sort
}