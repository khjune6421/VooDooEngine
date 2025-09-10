#include "WindowManager.h"

using namespace std;

vector<pair<UINT, WindowManager::WindowData>> WindowManager::g_windows = {};

LRESULT WindowManager::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);

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

			return 0;
		}

		return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool WindowManager::ProcessMessage()
{
	MSG msg = {};

	while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return msg.message != WM_QUIT;
}

void WindowManager::CreateWindowData(std::wstring className, std::wstring windowName, LONG width, LONG height, int amount)
{
	for (int i = 0; i < amount; i++)
	{
		WindowData windowData
		{
			className + to_wstring(i),
			windowName + to_wstring(i),
			width,
			height
		};
		g_windows.emplace_back(i, windowData);
	}
}

void WindowManager::InitializeAllWindows(HINSTANCE hInstance, int nShowCmd)
{
	for (auto& window : g_windows)
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
			window.second.className.c_str(),
			nullptr
		};
		if (!RegisterClassEx(&wc))
		{
			MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK);
			continue;
		}

		RECT rect = { 0, 0, window.second.width, window.second.height };
		window.second.hWnd = CreateWindow
		(
			window.second.className.c_str(),
			window.second.windowName.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			window.second.width, window.second.height,
			GetDesktopWindow(),
			nullptr,
			hInstance,
			nullptr
		);
		if (!window.second.hWnd)
		{
			MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK);
			continue;
		}

		ResizeWindow(window.second.hWnd, window.second.width, window.second.height);
		ShowWindow(window.second.hWnd, nShowCmd);
		UpdateWindow(window.second.hWnd);
		SetCursor(LoadCursorW(nullptr, IDC_ARROW));
		window.second.renderer = new VDRender(window.second.hWnd, window.second.width, window.second.height);
	}
}

void WindowManager::ResizeWindow(HWND hWnd, LONG width, LONG height)
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
}