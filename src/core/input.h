#pragma once
#include <windows.h>
#include <cstdint>

class Input {
public:
    enum MouseButton : int8_t {
        Left,
        Right,
        Middle,
        X1,
        X2,
        TOTAL
    };

    Input();

    bool HookWndProc(HWND hWnd);
    void UnhookWndProc(HWND hWnd);

    void Reset();

    bool IsPressed(int vk) const;
    bool IsJustPressed(int vk) const;
    bool IsReleased(int vk) const;

    bool IsMouseDown(MouseButton btn) const { return mouseDown[btn]; }
    bool IsMousePressed(MouseButton btn) const { return mousePressed[btn]; }
    bool IsMouseReleased(MouseButton btn) const { return mouseReleased[btn]; }

    float GetScrollDelta() const { return scrollDelta; }

private:
    static Input* instance;
    static LONG_PTR origWndProc;

    static LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Update(UINT uMsg, WPARAM wParam);

    bool keyDown[256] = {};
    bool keyPressed[256] = {};
    bool keyReleased[256] = {};

    bool mouseDown[TOTAL] = {};
    bool mousePressed[TOTAL] = {};
    bool mouseReleased[TOTAL] = {};

    float scrollDelta = 0.0f;
};