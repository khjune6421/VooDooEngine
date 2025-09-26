#include "Engine.h"

#include "TestScene.h"

using namespace std;

unordered_map<wstring, function<unique_ptr<Scene>()>> VDGM::g_sceneFactory =
{
	{ L"TestScene", []() { return make_unique<TestScene>(L"../Assets/ObjectPos/Trees.dat"); } },
	//{ L"TestScene", []() { return make_unique<TestScene>("Trees.dat"); } },
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
	HWND a = VDW::CreateWindowAndRenderer(L"VooDoo Class1", L"VooDoo Engine1", 1280, 720);
	// This works in a way I don't understand // creates static vertex buffer with one m_device but somewhow it's usable in other m_device instances
	//VDW::g_windows[a]->ChangeShader(PixelShaders::ColorShift);
	//VDW::g_windows[a]->ChangeState();
	//VDW::g_windows[a]->ChangeState();

	//HWND b = VDW::CreateWindowAndRenderer(L"VooDoo Class2", L"VooDoo Engine2", 640, 480);
	//VDW::g_windows[b]->ChangeShader(PixelShaders::Greyscale);
	//VDW::g_windows[b]->ChangeState();
	//VDW::g_windows[b]->ChangeState();

	//VDW::CreateWindowAndRenderer(L"VooDoo Class3", L"VooDoo Engine3", 1280, 720);

	// Create and set the test scene
	VDGM::g_currentScene = make_unique<TestScene>(L"../Assets/ObjectPos/Trees.dat");
	//VDGM::g_currentScene = make_unique<TestScene>("Trees.dat");

	while (VDW::ProcessMessage())
	{
		VDGM::GameLoop();
		if (GetAsyncKeyState(VK_TAB) & 0x0001) VDGM::ChangeScene(L"TestScene");
	}

	VDW::g_windows.clear();
	for (auto& render : VDW::g_renders) delete render;

	return 0;
}