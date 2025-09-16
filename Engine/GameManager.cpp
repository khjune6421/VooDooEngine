#include "GameManager.h"

using namespace std;

vector<Object*> g_objects;

namespace VDGM
{
	unordered_map<wstring, unique_ptr<Scene>> g_sceneMap;
	unique_ptr<Scene> g_currentScene = nullptr;

	float g_deltaTimeF = 0.0f;
	double g_deltaTimeD = 0.0;

	void ChangeScene(const std::wstring& sceneName)
	{
		if (g_sceneMap.find(sceneName) != g_sceneMap.end())
		{
			g_currentScene = std::move(g_sceneMap[sceneName]);
			g_sceneMap.erase(sceneName);
		}
		else
		{
			MessageBoxW(nullptr, L"Scene not found", L"Error", MB_OK);
		}
	}

	void GameLoop()
	{
		g_deltaTimeD = GetdeltaTime();
		g_deltaTimeF = static_cast<float>(g_deltaTimeD);

		Update(g_deltaTimeF);
		Render();
	}

	double GetdeltaTime()
	{
		static ULONGLONG previousTime = GetTickCount64();
		ULONGLONG currentTime = GetTickCount64();
		double deltaTime = static_cast<double>(currentTime - previousTime) / 1000.0;
		previousTime = currentTime;
		return deltaTime;
	}

	void Update(float deltaTime)
	{
		if (g_currentScene) g_currentScene->Update(deltaTime);
	}

	void Render()
	{
		for (auto& render : VDW::g_renders) render->SceneRender();
	}
}