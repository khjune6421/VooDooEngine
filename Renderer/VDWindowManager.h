#pragma once

#include "VDRender.h"

namespace VDW
{
	extern HINSTANCE g_hInstance;
	extern std::vector<VDRender*> g_renders;
	extern std::unordered_map<HWND, VDRender*> g_windows;

	LRESULT CALLBACK Wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool ProcessMessage();

	void CreateWindowAndRenderer(std::wstring className, std::wstring windowName, LONG width = 1280, LONG height = 720);
	void ResizeWindow(HWND hWnd, LONG width, LONG height);
}