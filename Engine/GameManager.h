#pragma once

#include "WindowManager.h"

namespace VDGM
{
	extern std::unique_ptr<Scene> g_currentScene;
	extern std::unordered_map<std::wstring, std::function<std::unique_ptr<Scene>()>> g_sceneFactory;

	extern float g_deltaTimeF;
	extern double g_deltaTimeD;

	void ChangeScene(const std::wstring& sceneName);

	bool GameLoop();

	double GetdeltaTime();

	void Update(float deltaTime);
	void Render();
}