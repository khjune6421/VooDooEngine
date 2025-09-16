#pragma once

#include "WindowManager.h"
#include "Scene.h"

namespace VDGM
{
	extern std::unordered_map<std::wstring, std::unique_ptr<Scene>> g_sceneMap;
	extern std::unique_ptr<Scene> g_currentScene;

	extern float g_deltaTimeF;
	extern double g_deltaTimeD;

	void ChangeScene(const std::wstring& sceneName);

	void GameLoop();

	double GetdeltaTime();

	void Update(float deltaTime);
	void Render();
}