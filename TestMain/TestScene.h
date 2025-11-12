#pragma once
#include "pch.h"

#include "Player.h"
#include "Windmill.h"
#include "Tree.h"

class TestScene : public Scene
{
	DirectX::XMFLOAT3 m_dawnColor = { 0.2f, 0.2f, 0.5f };
	DirectX::XMFLOAT3 m_morningColor = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_noonColor = { 0.5f, 0.25f, 0.25f };
	DirectX::XMFLOAT3 m_nightColor = { 0.1f, 0.1f, 0.25f };
	enum TimeOfDay
	{
		Dawn,
		Morning,
		Noon,
		Night,

		TimeOfDayCount
	} m_timeOfDay = Noon;
	DirectX::XMFLOAT3 m_timeColors[TimeOfDayCount] =
	{
		m_morningColor,
		m_noonColor,
		m_nightColor
	};

	std::unique_ptr<Player> m_player = nullptr;
	std::unique_ptr<Windmill> m_windmill = std::make_unique<Windmill>(this);
	std::unique_ptr<Object> m_camera = std::make_unique<Object>(this);

	std::unique_ptr<Object> m_lightObj = nullptr;
	std::unique_ptr<Object> m_torch = nullptr;

public:
	TestScene();

	void Update(float deltaTime) override;
};