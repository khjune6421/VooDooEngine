#pragma once

#include "Scene.h"

class TestScene : public Scene
{
public:
	TestScene(std::string dataFile);
	~TestScene() override = default;
	void Update(float deltaTime) override;
};