#include "Engine.h"

#include "TestScene.h"

using namespace std;

unordered_map<wstring, function<unique_ptr<Scene>()>> VDGM::g_sceneFactory =
{
	{ L"TestScene", []() { return make_unique<TestScene>(L"../Assets/ObjectPos/Trees.dat"); } }
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

	// Create multiple windows with different shaders
	VDW::CreateWindowAndRenderer(L"VooDoo Class1", L"VooDoo Engine1", 1280, 720);
	VDW::CreateWindowAndRenderer(L"VooDoo Class2", L"VooDoo Engine2", 640, 480);
	VDW::CreateWindowAndRenderer(L"VooDoo Class3", L"VooDoo Engine3", 1280, 480);
	VDW::CreateWindowAndRenderer(L"VooDoo Class4", L"VooDoo Engine4", 640, 960);
	VDW::CreateWindowAndRenderer(L"VooDoo Class5", L"VooDoo Engine5", 1280, 720);

	// Create and set the test scene
	VDGM::g_currentScene = make_unique<TestScene>(L"../Assets/ObjectPos/Trees.dat");

	while (VDGM::GameLoop())
	{
		if (GetAsyncKeyState(VK_TAB) & 0x0001) VDGM::ChangeScene(L"TestScene");
	}

	VDW::g_windows.clear();
	for (auto& render : VDW::g_renders) delete render;

	return 0;
}