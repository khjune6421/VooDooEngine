#include "pch.h"
#include "PathFindingScene.h"

using namespace std;
using namespace DirectX;

void PathFindingScene::CreateRandomObstacles()
{
	for (int x = 0; x < GRID_SIZE; ++x)
	{
		for (int z = 0; z < GRID_SIZE; ++z)
		{
			m_grid[x][z] = make_unique<Node>(this);
			m_grid[x][z]->SetPosition
			(
				XMVECTOR
				{
					GridPosToWorldPos(x),
					1.0f,
					GridPosToWorldPos(z),
					1.0f
				}
			);
			float random = static_cast<float>(rand() % 100) / 100.0f;
			if (random < 0.0f) m_grid[x][z]->m_isWalkable = false;
			else m_grid[x][z]->m_isActive = false;
		}
	}
}

float PathFindingScene::GridPosToWorldPos(int gridPos)
{
	return (static_cast<float>(gridPos) - GRID_SIZE / 2.0f) * 2.0f + 1.0f;
}

int PathFindingScene::WorldPosToGridPos(float worldPos)
{
	return static_cast<int>((worldPos + GRID_SIZE) / 2.0f);
}

deque<pair<int, int>> PathFindingScene::FindPath(pair<int, int> start, pair<int, int> end)
{
	const enum DirEnum
	{
		Up,
		Left,
		Down,
		Right,

		Count
	};
	constexpr pair<int, int> dir[Count] =
	{
		{ -1, 0 },
		{ 0, -1 },
		{ 1, 0 },
		{ 0, 1 }
	};

	for (auto& x : m_grid) for (const auto& z : x)
	{
		z->m_pathCost = 0;
		z->m_isVisited = false;
	}

	pair<int, int> pos = start;
	m_grid[start.first][start.second]->m_isVisited = true;
	m_grid[start.first][start.second]->m_pathCost = 1;
	deque<pair<int, int>> posQue;
	posQue.push_back(pos);

	vector<vector<pair<int, int>>> parent(GRID_SIZE, vector<pair<int, int>>(GRID_SIZE, { -1, -1 }));

	while (!posQue.empty())
	{
		pos = posQue.front();
		if (pos.first == end.first && pos.second == end.second) break;
		posQue.pop_front();

		for (auto& i : dir)
		{
			int dirX = pos.first + i.first;
			if (dirX < 0 || dirX >= GRID_SIZE) continue;
			int dirY = pos.second + i.second;
			if (dirY < 0 || dirY >= GRID_SIZE) continue;

			if (!m_grid[dirX][dirY]->m_isVisited && m_grid[dirX][dirY]->m_isWalkable)
			{
				m_grid[dirX][dirY]->m_isVisited = true;
				int newCost = m_grid[pos.first][pos.second]->m_pathCost + 1;

				if (m_grid[dirX][dirY]->m_pathCost == 0 || m_grid[dirX][dirY]->m_pathCost > newCost)
				{
					m_grid[dirX][dirY]->m_pathCost = newCost;
				}

				parent[dirX][dirY] = pos;
				posQue.emplace_back(dirX, dirY);
			}
		}
	}

	deque<pair<int, int>> path;
	if (m_grid[end.first][end.second]->m_isVisited)
	{
		pair<int, int> current = end;
		while (current.first != -1 && current.second != -1)
		{
			path.push_back(current);
			current = parent[current.first][current.second];
		}
		reverse(path.begin(), path.end());
	}

	return path;
}

PathFindingScene::PathFindingScene()
{
	m_camera->SetPosition(XMVECTOR{ 0.0f, 40.0f, -60.0f, 1.0f });
	m_camera->LookAt(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });
	m_camera->AddComponent<Camera>(3400, 1440, 0.1f, 200.0f, XM_PIDIV4);
	m_ambientFog.w = 200.0f;

	m_directionalLight.color = XMFLOAT4{ 0.8f, 0.8f, 0.8f, 1.0f };
	m_directionalLight.direction = XMVector3Normalize(XMVECTOR{ 1.0f, -1.0f, 1.0f, 0.0f });

	m_camRotator->AddChild(m_camera.get());

	unique_ptr<Object> plane = make_unique<Object>(this);
	plane->AddComponent<Shape>(L"Shape_Plane", L"VertexShader", L"PixelShader", L"Plain");
	plane->SetScale(XMFLOAT3{ 25.0f, 1.0f, 25.0f });
	m_objects.emplace_back(move(plane));

	m_player = make_unique<Object>(this);
	m_player->AddComponent<Shape>(L"Player", L"VertexShader", L"PixelShader", L"Lilypads");
	m_player->AddComponent<PointLight>(XMFLOAT3{ 1.0f, 0.75f, 0.5f }, 10.0f);
	m_player->SetPosition
	(
		XMVECTOR
		{
			GridPosToWorldPos(0),
			1.0f,
			GridPosToWorldPos(0),
			1.0f
		}
	);

	CreateRandomObstacles();
}

void PathFindingScene::Update(float deltaTime)
{
	if (GetAsyncKeyState('W') & 0x8000) m_camRotator->Rotate(XMVECTOR{ -1.0f * deltaTime, 0.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('S') & 0x8000) m_camRotator->Rotate(XMVECTOR{ 1.0f * deltaTime, 0.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('A') & 0x8000) m_camRotator->Rotate(XMVECTOR{ 0.0f, -1.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState('D') & 0x8000) m_camRotator->Rotate(XMVECTOR{ 0.0f, 1.0f * deltaTime, 0.0f, 0.0f });

	if (GetAsyncKeyState(VK_SPACE) & 0x0001) m_isPlaceMode = !m_isPlaceMode;

	m_camera->Update(deltaTime);
	m_player->Update(deltaTime);
	m_playerGridPos =
	{
		WorldPosToGridPos(XMVectorGetX(m_player->GetWorldPosition())),
		WorldPosToGridPos(XMVectorGetZ(m_player->GetWorldPosition()))
	};

	if (!m_currentPath.empty())
	{
		pair<int, int> nextPos = m_currentPath.front();
		XMVECTOR targetPos =
		{
			GridPosToWorldPos(nextPos.first),
			1.0f,
			GridPosToWorldPos(nextPos.second),
			1.0f
		};
		XMVECTOR direction = XMVectorSubtract(targetPos, m_player->GetWorldPosition());
		float distance = XMVectorGetX(XMVector3Length(direction));
		direction = XMVector3Normalize(direction);
		float moveSpeed = 10.0f;
		if (distance < moveSpeed * deltaTime)
		{
			m_player->SetPosition(targetPos);
			m_currentPath.pop_front();
		}
		else
		{
			XMVECTOR newPos = XMVectorAdd(m_player->GetWorldPosition(), XMVectorScale(direction, moveSpeed * deltaTime));
			m_player->SetPosition(newPos);
		}
	}

	for (auto& x : m_grid) for (const auto& z : x) z->Update(deltaTime);

	Scene::Update(deltaTime);
}

void PathFindingScene::Raycast(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayEnd)
{
	XMVECTOR planePoint = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR planeNormal = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR plane = XMPlaneFromPointNormal(planePoint, planeNormal);

	XMVECTOR rayDirection = XMVector3Normalize(XMVectorSubtract(rayEnd, rayOrigin));
	float dotProduct = XMVectorGetX(XMVector3Dot(rayDirection, planeNormal));

	if (abs(dotProduct) < 0.0001f) { return; } // Parallel to the plane

	XMVECTOR hit = XMPlaneIntersectLine(plane, rayOrigin, rayEnd);

	if (XMVector3IsInfinite(hit)) { return; } // No intersection

	float hitX = XMVectorGetX(hit);
	float hitZ = XMVectorGetZ(hit);

	int gridX = WorldPosToGridPos(hitX);
	int gridZ = WorldPosToGridPos(hitZ);
	if (gridX < 0 || gridX >= GRID_SIZE || gridZ < 0 || gridZ >= GRID_SIZE) return;

	if (m_isPlaceMode)
	{
		m_grid[gridX][gridZ]->m_isActive = !m_grid[gridX][gridZ]->m_isActive;
		m_grid[gridX][gridZ]->m_isWalkable = !m_grid[gridX][gridZ]->m_isWalkable;
	}
	else
	{
		pair<int, int> targetPos = { gridX, gridZ };
		if (m_playerGridPos == targetPos) return;
		if (!m_grid[gridX][gridZ]->m_isWalkable) return;

		m_currentPath = FindPath(m_playerGridPos, targetPos);
	}
}