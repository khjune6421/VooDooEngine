#include "GameManager.h"

using namespace std;

namespace VDGM
{
	unique_ptr<Scene> g_currentScene = nullptr;

	vector<Object*> g_objects;

	float g_deltaTimeF = 0.0f;
	double g_deltaTimeD = 0.0;

	void ChangeScene(const std::wstring& sceneName)
	{
		auto it = g_sceneFactory.find(sceneName);
		if (it != g_sceneFactory.end())
		{
			g_currentScene.reset();
			g_currentScene = it->second();
		}
		else MessageBoxW(nullptr, (L"Scene " + sceneName + L" not found").c_str(), L"Error", MB_OK);
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