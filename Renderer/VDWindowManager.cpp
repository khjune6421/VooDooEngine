#include "VDWindowManager.h"

using namespace std;

HINSTANCE VDW::g_hInstance = nullptr;
vector<VDRender*> VDW::g_renders = {};
unordered_map<HWND, VDRender*> VDW::g_windows = {};

LRESULT CALLBACK VDW::Wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool isDragging = false;
	static POINT lastMousePos = {};

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
			g_windows[hWnd]->SetViewport(static_cast<float>(rect.left), static_cast<float>(rect.top));
		}
		return 0;

	case WM_LBUTTONDOWN:
		isDragging = true;
		GetCursorPos(&lastMousePos);
		SetCapture(hWnd);
		return 0;

	case WM_LBUTTONUP:
		isDragging = false;
		ReleaseCapture();
		return 0;

	case WM_MOUSEMOVE:
		if (isDragging)
		{
			POINT currentMousePos = {};
			GetCursorPos(&currentMousePos);

			int deltaX = currentMousePos.x - lastMousePos.x;
			int deltaY = currentMousePos.y - lastMousePos.y;

			RECT rect = {};
			GetWindowRect(hWnd, &rect);

			int newLeft = rect.left + deltaX;
			int newTop = rect.top + deltaY;

			SetWindowPos
			(
				hWnd, nullptr,
				newLeft,
				newTop,
				0, 0,
				SWP_NOSIZE | SWP_NOZORDER
			);

			if (g_windows[hWnd]) g_windows[hWnd]->SetViewport(static_cast<float>(newLeft), static_cast<float>(newTop));

			lastMousePos = currentMousePos;
		}
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;

		case VK_F4:
			g_windows[hWnd]->ChangeState();
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

HWND VDW::CreateWindowAndRenderer(std::wstring className, std::wstring windowName, LONG width, LONG height)
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
		return nullptr;
	}

	HWND hWnd = CreateWindow
	(
		className.c_str(),
		windowName.c_str(),
		WS_POPUP,
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
		return nullptr;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetCursor(LoadCursorW(nullptr, IDC_ARROW));

	g_renders.push_back(new VDRender(hWnd, width, height));
	g_windows[hWnd] = g_renders.back();
	ResizeWindow(hWnd, width, height);

	return hWnd;
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
		hWnd, HWND_TOPMOST,
		oldRect.left, oldRect.top,
		newWidth, newHeight,
		SWP_SHOWWINDOW
	);

	MoveWindow(hWnd, oldRect.left, oldRect.top, newWidth, newHeight, TRUE);

	if (g_windows[hWnd])
	{
		g_windows[hWnd]->Resize(width, height);
		g_windows[hWnd]->SetViewport(static_cast<float>(oldRect.left), static_cast<float>(oldRect.top));
	}
}