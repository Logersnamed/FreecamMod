#include "core/input.h"
#include "utils/debug.h"
#include <iostream>

Input* Input::instance = nullptr;
LONG_PTR Input::origWndProc = 0;

Input::Input() {
	instance = this;
}

LRESULT __stdcall Input::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	instance->Update(uMsg, wParam);

	return CallWindowProcW((WNDPROC)Input::origWndProc, hWnd, uMsg, wParam, lParam);
}

bool Input::HookWndProc(HWND hWnd) {
    Logger::Info("Hooking WndProc...");
    if (!hWnd) {
		Logger::Error("Failed to hook WndProc: Invalid window handle");
        return false;
    }
    
    origWndProc = SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)Input::WndProc);
    return true;
}

void Input::UnhookWndProc(HWND hWnd) {
    Logger::Info("Unhooking WndProc...");
    if (!origWndProc || !hWnd) {
		Logger::Warn("Failed to unhook WndProc: No original WndProc or invalid window handle");
        return;
    }

    SetWindowLongPtrW(hWnd, GWLP_WNDPROC, origWndProc);
}

void Input::Update(UINT uMsg, WPARAM wParam) {
    switch (uMsg) {
        // Keyboard keys
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (!keyDown[wParam]) {
            keyPressed[wParam] = true;
        }
        keyDown[wParam] = true;
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        keyReleased[wParam] = true;
        keyDown[wParam] = false;
        break;

        // Mouse buttons
    case WM_LBUTTONDOWN:   if (!mouseDown[Left]) mousePressed[Left] = true; mouseDown[Left] = true; break;
    case WM_LBUTTONUP:     mouseReleased[Left] = true; mouseDown[Left] = false; break;
    case WM_RBUTTONDOWN:   if (!mouseDown[Right]) mousePressed[Right] = true; mouseDown[Right] = true; break;
    case WM_RBUTTONUP:     mouseReleased[Right] = true; mouseDown[Right] = false; break;
    case WM_MBUTTONDOWN:   if (!mouseDown[Middle]) mousePressed[Middle] = true; mouseDown[Middle] = true; break;
    case WM_MBUTTONUP:     mouseReleased[Middle] = true; mouseDown[Middle] = false; break;
    case WM_XBUTTONDOWN:
        if (HIWORD(wParam) == XBUTTON1) { if (!mouseDown[X1]) mousePressed[X1] = true; mouseDown[X1] = true; }
        if (HIWORD(wParam) == XBUTTON2) { if (!mouseDown[X2]) mousePressed[X2] = true; mouseDown[X2] = true; }
        break;
    case WM_XBUTTONUP:
        if (HIWORD(wParam) == XBUTTON1) { mouseReleased[X1] = true; mouseDown[X1] = false; }
        if (HIWORD(wParam) == XBUTTON2) { mouseReleased[X2] = true; mouseDown[X2] = false; }
        break;

        // Mouse wheel
    case WM_MOUSEWHEEL:
        scrollDelta += GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f;
		break;
    }
}

void Input::Reset() {
    memset(keyPressed, 0, sizeof(keyPressed));
    memset(keyReleased, 0, sizeof(keyReleased));
    memset(mousePressed, 0, sizeof(mousePressed));
    memset(mouseReleased, 0, sizeof(mouseReleased));
    scrollDelta = 0.0f;
}

bool Input::IsPressed(int vk) const {
    return keyDown[vk];
}

bool Input::IsJustPressed(int vk) const {
    return keyPressed[vk];
}

bool Input::IsReleased(int vk) const {
    return keyReleased[vk];
}
