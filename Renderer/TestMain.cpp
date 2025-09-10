#include "Device.h"
#include "Render.h"

#define DEVICE VDD::g_deviceInfo

constexpr LONG WINDOW_WIDTH = 1920;
constexpr LONG WINDOW_HEIGHT = 1080;

HWND g_hWnd = nullptr;
bool g_isRunning = true;

const wchar_t g_className[256] = L"VooDoo Class";
const wchar_t g_windowName[256] = L"VooDoo Engine";

void InitWindow(LONG width, LONG height, HINSTANCE hInstance = nullptr, int nShowCmd = SW_SHOW);
void ResizeWindow(HWND hWnd, LONG width, LONG height);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool ProcessMessage();

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#endif
{
#ifdef _DEBUG
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
#else
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, hInstance, nShowCmd);
#endif

	VDD::Initialize();
	VDR::LoadData();

	while (g_isRunning && ProcessMessage()) VDR::SceneRender();

	VDD::Release();
	VDR::ReleaseData();
}

void InitWindow(LONG width, LONG height, HINSTANCE hInstance, int nShowCmd)
{
	if (!hInstance) hInstance = GetModuleHandleW(nullptr);

	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		MsgProc,
		0, 0,
		hInstance,
		nullptr, nullptr,
		reinterpret_cast<HBRUSH>(GetStockObject(DKGRAY_BRUSH)),
		nullptr,
		g_className,
		nullptr
	};
	if (!RegisterClassEx(&wc))
	{
		MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK);
		return;
	}

	RECT rect = { 0, 0, width, height };
	g_hWnd = CreateWindow
	(
		g_className,
		g_windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		GetDesktopWindow(),
		nullptr,
		hInstance,
		nullptr
	);
	if (!g_hWnd)
	{
		MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK);
		return;
	}

	ResizeWindow(g_hWnd, width, height);

	ShowWindow(g_hWnd, nShowCmd);
	UpdateWindow(g_hWnd);
	SetCursor(LoadCursorW(nullptr, IDC_ARROW));
}

LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		g_isRunning = false;

		return 0;

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			LONG width = LOWORD(lParam);
			LONG height = HIWORD(lParam);
			if (width != 0 && height != 0) ResizeWindow(hWnd, width, height);
		}
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			g_isRunning = false;

			return 0;
		}

		return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void ResizeWindow(HWND hWnd, LONG width, LONG height)
{
	RECT oldRect = {};
	GetWindowRect(hWnd, &oldRect);
	DWORD style = static_cast<DWORD>(GetWindowLongW(hWnd, GWL_STYLE));
	DWORD exStyle = static_cast<DWORD>(GetWindowLongW(hWnd, GWL_EXSTYLE));

	RECT newRect = { 0, 0, width, height };

	AdjustWindowRectEx(&newRect, style, FALSE, exStyle);



	LONG newWidth = newRect.right - newRect.left;
	LONG newHeight = newRect.bottom - newRect.top;

	SetWindowPos
	(
		hWnd, nullptr,
		oldRect.left, oldRect.top,
		newWidth, newHeight,
		SWP_SHOWWINDOW
	);

	MoveWindow(hWnd, oldRect.left, oldRect.top, newWidth, newHeight, TRUE);

	if (DEVICE.device) VDD::Resize(static_cast<UINT>(width), static_cast<UINT>(height));
}

bool ProcessMessage()
{
	MSG msg = {};

	while(PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return msg.message != WM_QUIT;
}