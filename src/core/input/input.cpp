#include "core/input/input.h"

#include <iostream>

#include "utils/debug.h"

Input* Input::instance = nullptr;
LONG_PTR Input::origWndProc = 0;

Input::Input() {
	instance = this;
}

LRESULT __stdcall Input::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	instance->Update(hWnd, uMsg, wParam, lParam);

	return CallWindowProcW((WNDPROC)Input::origWndProc, hWnd, uMsg, wParam, lParam);
}

bool Input::HookWndProc(HWND hWnd) {
    Logger::Info("Hooking WndProc...");
    if (!hWnd) {
		Logger::Error("Failed to hook WndProc: Invalid window handle %p", hWnd);
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

void Input::Update(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int key = -1;

    switch (uMsg) {
        case WM_KILLFOCUS:
        case WM_ACTIVATEAPP:
            isWindowFocused = (uMsg == WM_ACTIVATEAPP);
            isWindowJustFocused = isWindowFocused;

            if (!wParam) {
                Reset();
                memset(keyDown, 0, sizeof(keyDown));
            }
            return;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            key = (int)wParam;
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            key = (int)wParam;
            break;

        case WM_LBUTTONDOWN: case WM_LBUTTONUP: key = VK_LBUTTON; break;
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: key = VK_RBUTTON; break;
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: key = VK_MBUTTON; break;
        case WM_XBUTTONDOWN: case WM_XBUTTONUP:
            key = (HIWORD(wParam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
            break;
    
        case WM_MOUSEMOVE: {
            if (!windowWidth || !windowHeight) {
                if (!GetWindowSize(hWnd)) break;
            }

            if (IsCursorVisible() || !isWindowFocused) break;

            mouseDeltaX = GET_X_LPARAM(lParam) - windowWidth / 2;
            mouseDeltaY = GET_Y_LPARAM(lParam) - windowHeight / 2;

            break;
        }
        case WM_SIZE:
            GetWindowSize(hWnd);
            break;

        case WM_MOUSEWHEEL:
            scrollDelta += GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f;
            return;
    }

    if (key == -1 || key >= 256) return;

    bool isDown = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN ||
        uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
        uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN);

    if (isDown) {
        if (!keyDown[key])
            keyPressed[key] = true;

        keyDown[key] = true;
    }
    else {
        keyReleased[key] = true;
        keyDown[key] = false;
    }
}

bool Input::IsCursorVisible() {
    CURSORINFO ci = {};
    ci.cbSize = sizeof(ci);
    return GetCursorInfo(&ci) && (ci.flags & CURSOR_SHOWING);
}

bool Input::GetWindowSize(HWND hWnd) {
    RECT rect;
    if (GetClientRect(hWnd, &rect)) {
        windowWidth = rect.right - rect.left;
        windowHeight = rect.bottom - rect.top;
        return true;
    }
    return false;
}

void Input::Reset() {
    memset(keyPressed, 0, sizeof(keyPressed));
    memset(keyReleased, 0, sizeof(keyReleased));
    scrollDelta = 0.0f;
    mouseDeltaX = 0;
    mouseDeltaY = 0;
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
