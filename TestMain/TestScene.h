#pragma once

#include "Engine.h"

#include "Tree.h"

class TestScene : public Scene
{
	std::unique_ptr<Object> m_camera = std::make_unique<Object>();

public:
	TestScene(std::wstring dataFile);

	void Update(float deltaTime) override;
};