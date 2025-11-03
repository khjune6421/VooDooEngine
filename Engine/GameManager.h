#pragma once

#include "WindowManager.h"

namespace VDGM
{
	extern std::unique_ptr<class Scene> g_currentScene;
	extern std::unordered_map<std::wstring, std::function<std::unique_ptr<class Scene>()>> g_sceneFactory;

	extern float g_deltaTime;

	void ChangeScene(const std::wstring& sceneName);

	bool GameLoop();

	float GetdeltaTime();

	void Update(float deltaTime);
	void Render();
}