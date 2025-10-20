#pragma once

#include "Engine.h"

#include "Player.h"
#include "Windmill.h"
#include "Tree.h"

class TestScene : public Scene
{
	DirectX::XMFLOAT3 m_morningColor = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_noonColor = { 0.5f, 0.25f, 0.25f };
	DirectX::XMFLOAT3 m_nightColor = { 0.1f, 0.1f, 0.25f };

	std::unique_ptr<Player> m_player = std::make_unique<Player>();
	std::unique_ptr<Windmill> m_windmill = std::make_unique<Windmill>();
	std::unique_ptr<Object> m_camera = std::make_unique<Object>();

	std::unique_ptr<Object> m_lightObj = nullptr;

public:
	TestScene(std::wstring dataFile);

	void Update(float deltaTime) override;
};