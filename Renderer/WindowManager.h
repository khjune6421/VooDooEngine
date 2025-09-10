#pragma once

#include "VDRender.h"

namespace WindowManager
{
	struct WindowData
	{
		std::wstring className;
		std::wstring windowName;
		LONG width = 0;
		LONG height = 0;

		HWND hWnd = nullptr;
		VDRender* renderer = nullptr;
	};
	extern std::vector<std::pair<UINT, WindowData>> g_windows;

	LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool ProcessMessage();

	void CreateWindowData(std::wstring className, std::wstring windowName, LONG width = 1280, LONG height = 720, int amount = 1);
	void InitializeAllWindows(HINSTANCE hInstance = nullptr, int nShowCmd = SW_SHOW);

	void ResizeWindow(HWND hWnd, LONG width, LONG height);
}