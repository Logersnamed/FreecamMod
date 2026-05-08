#include "core/input/input.h"

#include <iostream>

#include "utils/debug.h"

Input::Input() {
    instance = this;
}

LRESULT __stdcall Input::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    instance->UpdateKeyboard(hWnd, uMsg, wParam, lParam);

    return CallWindowProcW((WNDPROC)Input::origWndProc, hWnd, uMsg, wParam, lParam);
}

UINT WINAPI Input::hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader) {
    UINT orig = origGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

    if (instance->isShouldGetInput) {
        if (orig != (UINT)-1 && uiCommand == RID_INPUT && pData) {
            RAWINPUT* raw = (RAWINPUT*)pData;

            if (raw->header.dwType == RIM_TYPEMOUSE) {
                instance->mouseDelta.x += raw->data.mouse.lLastX;
                instance->mouseDelta.y += raw->data.mouse.lLastY;
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

void Input::UpdateGamepad() {
    prevState = state;

    ZeroMemory(&state, sizeof(XINPUT_STATE));
    if (XInputGetState(0, &state) != ERROR_SUCCESS) return;

    auto normalizeTrigger = [](BYTE trigger) -> float { return trigger / 255.0f; };
    auto normalizeStick = [](float2 stick) -> float2 {
        stick = stick / 32767.0f;

        const float deadzone = 0.1f;
        const float lenght = stick.length();
        if (lenght < deadzone) return float2(0);

        const float scale = (lenght - deadzone) / (lenght * (1.0f - deadzone));
        return stick * scale;
    };

    thumbLeft = normalizeStick(float2(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY));
    thumbRight = normalizeStick(float2(state.Gamepad.sThumbRX, -state.Gamepad.sThumbRY));
    leftTrigger = normalizeTrigger(state.Gamepad.bLeftTrigger);
    rightTrigger = normalizeTrigger(state.Gamepad.bRightTrigger);
}

void Input::UpdateKeyboard(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

std::vector<uint8_t> Input::GetReleasedNumkeysInOrder() {
    bool isAnyPressed = false;
    for (int key = 0; key < NUM_KEYS_COUNT; ++key) {
        int keyCode = key + (int)'0';

        if (IsReleased(keyCode)) {
            numRowKeys[key].wasRecentlyReleased = true;
        }

        if (IsPressed(keyCode)) {
            isAnyPressed = true;
            numRowKeys[key].wasRecentlyReleased = false;

            if (IsJustPressed(keyCode)) {
                numRowKeys[key].pressId = ++id;
            }
        }
    }

    if (isAnyPressed || id == 0) return {};

    std::vector<uint8_t> result;
    for (int key = 0; key < NUM_KEYS_COUNT; ++key) {
        if (numRowKeys[key].wasRecentlyReleased) {
            numRowKeys[key].wasRecentlyReleased = false;
            result.push_back(key);
        }
    }

    if (!result.empty()) {
        std::sort(result.begin(), result.end(),
            [this](uint8_t a, uint8_t b) {
                return numRowKeys[a].pressId < numRowKeys[b].pressId;
            }
        );
    }

    id = 0;

    return result;
}