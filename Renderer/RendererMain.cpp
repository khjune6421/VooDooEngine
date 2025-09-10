#include "WindowManager.h"

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#endif
{
	WindowManager::CreateWindowData(L"VooDoo Class", L"VooDoo Engine", 1280, 720, 3);

#ifdef _DEBUG
	WindowManager::InitializeAllWindows();
#else
	WindowManager::InitializeAllWindows(hInstance, nShowCmd);
#endif

	for (auto& window : WindowManager::g_windows)
	{
		window.second.renderer->ChangeShader(window.first % 3);
	}

	while (WindowManager::ProcessMessage())
	{
		for (auto& window : WindowManager::g_windows)
		{
			if (window.second.renderer) window.second.renderer->SceneRender();
		}
	}

	for (auto& window : WindowManager::g_windows)
	{
		if (window.second.renderer)
		{
			delete window.second.renderer;
		}
	}
}