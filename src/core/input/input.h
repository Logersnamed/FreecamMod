#pragma once
#include <windows.h>
#include <windowsx.h>
#include <cstdint>

#include "utils/types.h"

class Input {
    struct KeyState {
        bool down;
        bool pressed;
        bool released;

        void Update(bool isDown) {
            if (isDown) {
                if (!down) pressed = true;
                down = true;
            }
            else {
                released = true;
                down = false;
            }
        }
    };

public:
    Input();

    bool HookWndProc(HWND hWnd);
    void UnhookWndProc(HWND hWnd);

    void Reset();

    bool IsPressed(int vk) const { return keyStates[vk].down; }
    bool IsJustPressed(int vk) const { return keyStates[vk].pressed; }
    bool IsReleased(int vk) const { return keyStates[vk].released; }

    float GetScrollDelta() const { return scrollDelta; }
    int2 GetMouseDelta() const { return mouseDelta; }

    bool IsWindowJustGetFocused() { return isWindowJustFocused; }

private:
    static inline Input* instance = nullptr;
    static inline LONG_PTR origWndProc = 0;

    KeyState keyStates[256] = {};
    float scrollDelta = 0.0f;
    int2 mouseDelta = 0;

    int2 halfWindowSize = 0;
    bool isWindowFocused = true;
    bool isWindowJustFocused = true;

    static LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Update(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnWindowFocus(bool getFocused, WPARAM wParam);

    bool GetWindowSize(HWND hWnd);
    bool IsCursorVisible();
};