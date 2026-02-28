#pragma once
#include <iostream>
#include <windows.h>

// From https://github.com/M0rtale/Universal-WndProc-Hook
class UWPH {
public:
	static void HookWndProc(HWND hWnd);
	static void UnhookWndProc(HWND hWnd);

	static float ScrollDelta;
private:
	static LONG_PTR origWndProc;
	static LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};