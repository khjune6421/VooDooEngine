#pragma once

#include "Engine.h"

#include "Player.h"
#include "WindMill.h"
#include "Tree.h"

enum CameraModes
{
	FlyBy,
	Behind,
	Rear,
	Stationary
};

class TestScene : public Scene
{
	std::unique_ptr<Camera> m_camera = std::make_unique<Camera>();

	std::unique_ptr<Player> m_player;
	std::unique_ptr<WindMill> m_windmill;
	std::unique_ptr<WindMillWing> m_projectileWing;

	CameraModes m_cameraMode = CameraModes::FlyBy;

public:
	TestScene(std::wstring dataFile);
	~TestScene() override = default;

	void Update(float deltaTime) override;
};