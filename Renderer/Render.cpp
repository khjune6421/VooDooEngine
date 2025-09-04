#include "Render.h"
#include "Device.h"

using namespace DirectX;

const XMFLOAT4 g_clearColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

static void ShowInfo()
{
	VDR::ShowFrameRate();
	VDR::DisplayDeviceInfo();
}

void SceneRender()
{
	VDR::ClearBackBuffer(g_clearColor);
#ifdef _DEBUG
	ShowInfo();
#endif
	VDR::PresentBackBuffer();
}