#pragma once

#include "Render.h"

namespace VDW
{
	extern HINSTANCE g_hInstance;
	extern std::vector<Render*> g_renders;
	extern std::unordered_map<HWND, Render*> g_windows;

	LRESULT CALLBACK Wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool ProcessMessage();

	HWND CreateWindowAndRenderer
	(
		std::wstring className, std::wstring windowName,
		int width, int height,
		int posX = CW_USEDEFAULT, int posY = CW_USEDEFAULT,
		const wchar_t* resourcePath = nullptr
	);
	void ResizeWindow(HWND hWnd, LONG width, LONG height);
}