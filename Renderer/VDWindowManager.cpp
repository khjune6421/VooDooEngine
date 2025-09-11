#include "VDWindowManager.h"

using namespace std;

HINSTANCE VDW::g_hInstance = nullptr;
vector<VDRender*> VDW::g_renders = {};
unordered_map<HWND, VDRender*> VDW::g_windows = {};

LRESULT CALLBACK VDW::Wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

	case WM_MOVE:
		if (g_windows[hWnd])
		{
			RECT rect = {};
			GetWindowRect(hWnd, &rect);
			g_windows[hWnd]->SetViewport(rect.left, rect.top);
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

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool VDW::ProcessMessage()
{
	MSG msg = {};

	while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT) return false;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return true;
}

void VDW::CreateWindowAndRenderer(std::wstring className, std::wstring windowName, LONG width, LONG height)
{
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		Wndproc,
		0, 0,
		g_hInstance,
		nullptr, nullptr,
		reinterpret_cast<HBRUSH>(GetStockObject(DKGRAY_BRUSH)),
		nullptr,
		className.c_str(),
		nullptr
	};
	if (!RegisterClassExW(&wc))
	{
		MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK);
		return;
	}

	HWND hWnd = CreateWindow
	(
		className.c_str(),
		windowName.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		GetDesktopWindow(),
		nullptr,
		g_hInstance,
		nullptr
	);
	if (!hWnd)
	{
		MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK);
		return;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetCursor(LoadCursorW(nullptr, IDC_ARROW));

	g_renders.push_back(new VDRender(hWnd, width, height));
	g_windows[hWnd] = g_renders.back();
	ResizeWindow(hWnd, width, height);
}

void VDW::ResizeWindow(HWND hWnd, LONG width, LONG height)
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

	if (g_windows[hWnd])
	{
		g_windows[hWnd]->Resize(width, height);
		g_windows[hWnd]->SetViewport(oldRect.left, oldRect.top);
	}
}