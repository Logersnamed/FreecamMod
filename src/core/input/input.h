#pragma once
#include <windows.h>
#include <cstdint>

class Input {
public:
    Input();

    bool HookWndProc(HWND hWnd);
    void UnhookWndProc(HWND hWnd);

    void Reset();

    bool IsPressed(int vk) const;
    bool IsJustPressed(int vk) const;
    bool IsReleased(int vk) const;

    float GetScrollDelta() const { return scrollDelta; }

private:
    static Input* instance;
    static LONG_PTR origWndProc;

    static LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Update(UINT uMsg, WPARAM wParam);

    bool keyDown[256] = {};
    bool keyPressed[256] = {};
    bool keyReleased[256] = {};

    float scrollDelta = 0.0f;
};