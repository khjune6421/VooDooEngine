#pragma once

#include "Scene.h"

#include "Player.h"
#include "WindMill.h"

class TestScene : public Scene
{
	std::unique_ptr<Player> m_player;
	std::unique_ptr<WindMill> m_windmill;

public:
	TestScene(std::string dataFile);
	~TestScene() override = default;

	void Update(float deltaTime) override;
};