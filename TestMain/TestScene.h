#pragma once

#include "Engine.h"

#include "Player.h"
#include "WindMill.h"
#include "Tree.h"

enum CameraModes
{
	Default,
	BirdEye
};

class TestScene : public Scene
{
	float projectileLifeTime = 3.0f;

	std::unique_ptr<Camera> m_camera = std::make_unique<Camera>();

	std::unique_ptr<Player> m_player;
	std::unique_ptr<WindMill> m_windmill;
	std::unique_ptr<WindMillWing> m_projectileWing;

	CameraModes m_cameraMode = CameraModes::Default;

public:
	TestScene(std::wstring dataFile);
	~TestScene() override;

	void Update(float deltaTime) override;

	void Raycast(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayEnd) override;
};