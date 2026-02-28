#include "universal_wndproc_hook.h"
#include <iostream>

LONG_PTR UWPH::origWndProc = NULL;
float UWPH::ScrollDelta = 0;

LRESULT __stdcall UWPH::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
	std::cout << "Message: " << uMsg << std::endl;

	if (uMsg == WM_MOUSEWHEEL) {
		ScrollDelta += (float)GET_WHEEL_DELTA_WPARAM(wParam) * 0.008f;
	}

	return CallWindowProcW((WNDPROC)UWPH::origWndProc, hWnd, uMsg, wParam, lParam);
}

void UWPH::HookWndProc(HWND hWnd) {
	if (hWnd) origWndProc = SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)UWPH::WndProc);
}

void UWPH::UnhookWndProc(HWND hWnd) {
    if (origWndProc && hWnd) SetWindowLongPtrW(hWnd, GWLP_WNDPROC, origWndProc);
}