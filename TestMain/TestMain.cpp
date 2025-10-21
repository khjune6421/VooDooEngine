#include "Engine.h"

#include "TestScene.h"

using namespace std;

unordered_map<wstring, function<unique_ptr<Scene>()>> VDGM::g_sceneFactory =
{
	{ L"TestScene", []() { return make_unique<TestScene>(); } }
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
	VDW::CreateWindowAndRenderer(L"VooDoo Class1", L"VooDoo Engine1", 1280, 720);
	//VDW::CreateWindowAndRenderer(L"VooDoo Class2", L"VooDoo Engine2", 1280, 720, 1280, 0, L"../Assets/Imposter/");
	//VDW::CreateWindowAndRenderer(L"VooDoo Class3", L"VooDoo Engine1", 1280, 720, 1280, 720);
	//VDW::CreateWindowAndRenderer(L"VooDoo Class4", L"VooDoo Engine2", 1280, 720, 0, 720, L"../Assets/Imposter/");

	// Create and set the test scene
	VDGM::g_currentScene = make_unique<TestScene>();

	while (VDGM::GameLoop()) if (GetAsyncKeyState(VK_TAB) & 0x0001) VDGM::ChangeScene(L"TestScene");

	VDW::g_windows.clear();
	for (auto& render : VDW::g_renders) delete render;

	return 0;
}