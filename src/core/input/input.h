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

    using getRawInputData_t = UINT(WINAPI*)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
    static inline getRawInputData_t origGetRawInputData{};
    static UINT WINAPI hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);

private:
    static inline Input* instance = nullptr;
    static inline LONG_PTR origWndProc = 0;

    bool isShouldGetInput = false;

    KeyState keyStates[256] = {};
    float scrollDelta = 0.0f;
    int2 mouseDelta = 0;

    bool isWindowFocused = true;
    bool isWindowJustFocused = true;

    static LRESULT __stdcall hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Update(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnWindowFocus(bool getFocused, WPARAM wParam);

    bool IsCursorVisible();
};