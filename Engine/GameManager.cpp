#include "GameManager.h"

using namespace std;

namespace VDGM
{
	unique_ptr<Scene> g_currentScene = nullptr;

	float g_deltaTime = 0.0f;

	void ChangeScene(const wstring& sceneName)
	{
		auto it = g_sceneFactory.find(sceneName);
		if (it != g_sceneFactory.end())
		{
			g_currentScene.reset();
			g_currentScene = it->second();
		}
		else MessageBoxW(nullptr, (L"Scene " + sceneName + L" not found").c_str(), L"Error", MB_OK);
	}

	bool GameLoop()
	{
		if (!VDW::ProcessMessage()) return false;

		g_deltaTime = GetdeltaTime();

		Update(g_deltaTime);
		Render();

		return true;
	}

	float GetdeltaTime()
	{
		static ULONGLONG previousTime = GetTickCount64();
		ULONGLONG currentTime = GetTickCount64();
		float deltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;
		previousTime = currentTime;

		return deltaTime;
	}

	void Update(float deltaTime)
	{
		// TODO: See if this can be optimized with multithreading
		if (g_currentScene) g_currentScene->Update(deltaTime);
	}

	void Render()
	{
		// This too
		for (auto& render : VDW::g_renders) render->Render();
	}
}