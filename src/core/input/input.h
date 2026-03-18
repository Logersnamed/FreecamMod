#pragma once
#include <windows.h>
#include <windowsx.h>
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

    int GetMouseDeltaX() const { return mouseDeltaX; }
    int GetMouseDeltaY() const { return mouseDeltaY; }

    bool IsWindowJustGetFocused() { 
        bool isJustFocused = isWindowJustFocused;
        isWindowJustFocused = false;
        return isJustFocused;
    }

private:
    static Input* instance;
    static LONG_PTR origWndProc;

    static LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Update(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool GetWindowSize(HWND hWnd);
    bool IsCursorVisible();

    bool isWindowFocused = false;
    bool isWindowJustFocused = false;

    bool keyDown[256] = {};
    bool keyPressed[256] = {};
    bool keyReleased[256] = {};

    float scrollDelta = 0.0f;

    int mouseDeltaX = 0;
    int mouseDeltaY = 0;

    int windowWidth = 0;
    int windowHeight = 0;
};