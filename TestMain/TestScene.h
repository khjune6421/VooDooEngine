#pragma once

#include "Engine.h"

#include "Tree.h"

class TestScene : public Scene
{
	std::unique_ptr<Camera> m_camera = std::make_unique<Camera>();

public:
	TestScene(std::wstring dataFile);

	void Update(float deltaTime) override;
};