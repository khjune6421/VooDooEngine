#pragma once

#include "Scene.h"

class TestScene : public Scene
{
public:
	TestScene();
	~TestScene() override = default;
	void Update(float deltaTime) override;
};