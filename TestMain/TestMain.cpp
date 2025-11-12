#include "pch.h"

#include "TestScene.h"
#include "PathFindingScene.h"

using namespace std;

unordered_map<wstring, function<unique_ptr<Scene>()>> VDGM::g_sceneFactory =
{
	{ L"TestScene", []() { return make_unique<TestScene>(); } },
	{ L"PathFindingScene", []() { return make_unique<PathFindingScene>(); }  }
};

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#endif
{
#ifdef _DEBUG
	VDW::g_hInstance = GetModuleHandleW(nullptr);
#else
	VDW::g_hInstance = hInstance;
#endif

	// Create multiple windows with different asset paths
	VDW::CreateWindowAndRenderer(L"VooDoo Class1", L"VooDoo Engine1", 1280, 720, 0, 0);
	VDW::CreateWindowAndRenderer(L"VooDoo Class2", L"VooDoo Engine2", 1280, 720, 1280, 0, L"../Assets/Imposter/");
	VDW::CreateWindowAndRenderer(L"VooDoo Class3", L"VooDoo Engine1", 1280, 720, 1280, 720);
	VDW::CreateWindowAndRenderer(L"VooDoo Class4", L"VooDoo Engine2", 1280, 720, 0, 720, L"../Assets/Imposter/");
	//VDW::CreateWindowAndRenderer(L"VooDoo Class0", L"VooDoo Engine0", 1, 1, 0, 0);

	// Create and set the test scene
	VDGM::g_currentScene = make_unique<PathFindingScene>();

	while (VDGM::GameLoop())
	{
		if (GetAsyncKeyState('1') & 0x0001) VDGM::ChangeScene(L"TestScene");
		else if (GetAsyncKeyState('2') & 0x0001) VDGM::ChangeScene(L"PathFindingScene");
	}

	VDW::g_windows.clear();
	for (auto& renderer : VDW::g_renderers) delete renderer;

	return 0;
}