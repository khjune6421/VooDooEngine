#pragma once
#include "Scene.h"

#include "Node.h"

class PathFindingScene : public Scene
{
	constexpr static int GRID_SIZE = 25;
	std::unique_ptr<Node> m_grid[GRID_SIZE][GRID_SIZE] = {};

	std::unique_ptr<Object> m_camera = std::make_unique<Object>(this);
	std::unique_ptr<Object> m_camRotator = std::make_unique<Object>(this);

	std::unique_ptr<Object> m_player = nullptr;
	std::pair<int, int> m_playerGridPos = { 0, 0 };
	std::deque<std::pair<int, int>> m_currentPath = {};

	bool m_isPlaceMode = true;

	void CreateRandomObstacles();
	float GridPosToWorldPos(int gridPos);
	int WorldPosToGridPos(float worldPos);

	std::deque<std::pair<int, int>> FindPath(std::pair<int, int> start, std::pair<int, int> end);

public:
	PathFindingScene();
	void Update(float deltaTime) override;
	void Raycast(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayEnd) override;
};