#pragma once

#include "Engine.h"

#include "Player.h"
#include "Windmill.h"
#include "Tree.h"

class TestScene : public Scene
{
	std::unique_ptr<Player> m_player = std::make_unique<Player>();
	std::unique_ptr<Windmill> m_windmill = std::make_unique<Windmill>();
	std::unique_ptr<Object> m_camera = std::make_unique<Object>();

public:
	TestScene(std::wstring dataFile);

	void Update(float deltaTime) override;
};