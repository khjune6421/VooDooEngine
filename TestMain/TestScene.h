#pragma once

#include "Scene.h"

class TestScene : public Scene
{
	std::unique_ptr<Object> m_player;

public:
	TestScene(std::string dataFile);
	~TestScene() override = default;
	void Update(float deltaTime) override;
};