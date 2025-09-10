#include "VDRender.h"

constexpr LONG WINDOW_WIDTH = 1920;
constexpr LONG WINDOW_HEIGHT = 1080;

HWND g_hWnd1 = nullptr;
HWND g_hWnd2 = nullptr;

std::unique_ptr<VDRender> g_renderer1 = nullptr;
std::unique_ptr<VDRender> g_renderer2 = nullptr;

bool g_isRunning = true;

const wchar_t g_className1[256] = L"VooDoo Class";
const wchar_t g_windowName1[256] = L"VooDoo Engine";

const wchar_t g_className2[256] = L"VooDoo Class 2";
const wchar_t g_windowName2[256] = L"VooDoo Engine 2";

HWND InitWindow(const wchar_t* className, const wchar_t* windowName, LONG width, LONG height, HINSTANCE hInstance = nullptr, int nShowCmd = SW_SHOW);
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
	g_hWnd1 = InitWindow(g_className1, g_windowName1, WINDOW_WIDTH, WINDOW_HEIGHT);
	g_hWnd2 = InitWindow(g_className2, g_windowName2, WINDOW_WIDTH, WINDOW_HEIGHT);
#else
	g_hWnd1 = InitWindow(g_className1, g_windowName1, WINDOW_WIDTH, WINDOW_HEIGHT, hInstance, nShowCmd);
	g_hWnd2 = InitWindow(g_className2, g_windowName2, WINDOW_WIDTH, WINDOW_HEIGHT, hInstance, nShowCmd);
#endif

	g_renderer1 = std::make_unique<VDRender>(g_hWnd1, WINDOW_WIDTH, WINDOW_HEIGHT);
	g_renderer2 = std::make_unique<VDRender>(g_hWnd2, WINDOW_WIDTH, WINDOW_HEIGHT);

	while (g_isRunning && ProcessMessage())
	{
		g_renderer1->SceneRender();
		g_renderer2->SceneRender();
	}
}

HWND InitWindow(const wchar_t* className, const wchar_t* windowName, LONG width, LONG height, HINSTANCE hInstance, int nShowCmd)
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
		className,
		nullptr
	};
	if (!RegisterClassEx(&wc))
	{
		MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK);
		return nullptr;
	}

	RECT rect = { 0, 0, width, height };
	HWND hWnd = CreateWindow
	(
		className,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		GetDesktopWindow(),
		nullptr,
		hInstance,
		nullptr
	);
	if (!hWnd)
	{
		MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK);
		return nullptr;
	}

	ResizeWindow(hWnd, width, height);

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);
	SetCursor(LoadCursorW(nullptr, IDC_ARROW));

	return hWnd;
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

	//if (g_renderer) g_renderer->Resize(static_cast<UINT>(width), static_cast<UINT>(height));
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