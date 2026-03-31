#include "core/input/input.h"

#include <iostream>

#include "utils/debug.h"

Input::Input() {
	instance = this;
}

LRESULT __stdcall Input::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	instance->Update(hWnd, uMsg, wParam, lParam);

	return CallWindowProcW((WNDPROC)Input::origWndProc, hWnd, uMsg, wParam, lParam);
}

UINT WINAPI Input::hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader) {
    UINT orig = origGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

    if (Input::instance->isShouldGetInput) {
        if (orig != (UINT)-1 && uiCommand == RID_INPUT && pData) {
            RAWINPUT* raw = (RAWINPUT*)pData;

            if (raw->header.dwType == RIM_TYPEMOUSE) {
                Input::instance->mouseDelta.x += raw->data.mouse.lLastX;
                Input::instance->mouseDelta.y += raw->data.mouse.lLastY;
            }
        }
    }

    return orig;
}

bool Input::HookWndProc(HWND hWnd) {
    LOG_INFO("Hooking WndProc...");
    if (!hWnd) {
        LOG_ERROR("Failed to hook WndProc: Invalid window handle %p", hWnd);
        return false;
    }
    
    origWndProc = SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)Input::hkWndProc);
    return true;
}

void Input::UnhookWndProc(HWND hWnd) {
    LOG_INFO("Unhooking WndProc...");
    if (!origWndProc || !hWnd) {
		LOG_WARN("Failed to unhook WndProc: No original WndProc or invalid window handle");
        return;
    }

    SetWindowLongPtrW(hWnd, GWLP_WNDPROC, origWndProc);
}

void Input::Update(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    isShouldGetInput = !IsCursorVisible() && isWindowFocused;

    int key = -1;
    switch (uMsg) {
        case WM_KILLFOCUS:
        case WM_ACTIVATEAPP:
            OnWindowFocus(uMsg == WM_ACTIVATEAPP, wParam);
            return;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
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

        case WM_MOUSEWHEEL:
            scrollDelta += (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            return;
    }

    if (key == -1 || key >= 256) return;

    bool isDown = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN ||
        uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
        uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN);

    keyStates[key].Update(isDown);
}

void Input::OnWindowFocus(bool getFocused, WPARAM wParam) {
    isWindowFocused = getFocused;
    isWindowJustFocused = getFocused;

    if (!wParam) {
        Reset();
        for (KeyState& key : keyStates) {
            key.down = false;
        }
    }
}

bool Input::IsCursorVisible() {
    CURSORINFO ci = {};
    ci.cbSize = sizeof(ci);
    return GetCursorInfo(&ci) && (ci.flags & CURSOR_SHOWING);
}

void Input::Reset() {
    for (KeyState& key : keyStates) {
        key.pressed = false;
        key.released = false;
    }
    scrollDelta = 0.0f;
    mouseDelta = 0;
    isWindowJustFocused = false;
}