#include "VDWindowManager.h"

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

	VDW::CreateWindowAndRenderer(L"VooDoo Class1", L"VooDoo Engine1", 1280, 720);
	VDW::CreateWindowAndRenderer(L"VooDoo Class2", L"VooDoo Engine2", 1280, 720);
	VDW::CreateWindowAndRenderer(L"VooDoo Class3", L"VooDoo Engine3", 1280, 720);
	VDW::CreateWindowAndRenderer(L"VooDoo Class4", L"VooDoo Engine4", 1280, 720);
	VDW::CreateWindowAndRenderer(L"VooDoo Class5", L"VooDoo Engine5", 1280, 720);

	while (VDW::ProcessMessage()) for (auto& render : VDW::g_renders) render->SceneRender();

	for (auto& render : VDW::g_renders) delete render;

	return 0;
}