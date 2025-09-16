#include "GameManager.h"
#include "TestScene.h"

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
	HWND a = VDW::CreateWindowAndRenderer(L"VooDoo Class1", L"VooDoo Engine1", 640, 480);
	VDW::g_windows[a]->CreateShapeVertexBuffer();
	VDW::g_windows[a]->ChangeShader(1);
	HWND b = VDW::CreateWindowAndRenderer(L"VooDoo Class2", L"VooDoo Engine2", 640, 480);
	VDW::g_windows[b]->ChangeShader(2);
	VDW::CreateWindowAndRenderer(L"VooDoo Class3", L"VooDoo Engine3", 640, 480);
	VDW::CreateWindowAndRenderer(L"VooDoo Class4", L"VooDoo Engine4", 1280, 720);

	// Create and set the test scene
	VDGM::g_sceneMap[L"TestScene"] = std::make_unique<TestScene>("../Assets/ObjectPos/Trees.dat");
	VDGM::ChangeScene(L"TestScene");

	while (VDW::ProcessMessage())
	{
		VDGM::GameLoop();
	}

	VDW::g_windows.clear();
	for (auto& render : VDW::g_renders) delete render;

	return 0;
}