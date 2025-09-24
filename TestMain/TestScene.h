#pragma once

#include "Engine.h"

#include "Player.h"
#include "WindMill.h"

class TestScene : public Scene
{
	std::unique_ptr<Player> m_player;
	std::unique_ptr<WindMill> m_windmill;
	std::unique_ptr<WindMillWing> m_projectileWing;

public:
	TestScene(std::wstring dataFile);
	~TestScene() override = default;

	void Update(float deltaTime) override;
};