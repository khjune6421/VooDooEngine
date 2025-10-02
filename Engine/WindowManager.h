#pragma once

#include "Render.h"

namespace VDW
{
	extern HINSTANCE g_hInstance;
	extern std::vector<Render*> g_renders;
	extern std::unordered_map<HWND, Render*> g_windows;

	LRESULT CALLBACK Wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool ProcessMessage();

	HWND CreateWindowAndRenderer(std::wstring className, std::wstring windowName, LONG width = 1280, LONG height = 720, const wchar_t* resourcePath = nullptr);
	void ResizeWindow(HWND hWnd, LONG width, LONG height);
}