#pragma once
#include <windows.h>
#include <windowsx.h>
#include <Xinput.h>
#include <cstdint>
#include <array>
#include <algorithm>

#include "core/config/con_var.h"
#include "utils/types.h"

#pragma comment(lib, "Xinput9_1_0.lib")

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

    ConVar<bool> disableController{ "input", "disable_controller", false };
    ConVar<bool> disableKeyboard{ "input", "disable_keyboard", false };

public:
    static constexpr int NUM_KEYS_COUNT = 10;
    using ReleasedNumkeys = FixedVec<uint8_t, NUM_KEYS_COUNT>;

    Input() { instance = this; }
    ~Input() { instance = nullptr; }

    static Input* GetInstance() { return instance; }

    bool HookWndProc(HWND hWnd);
    void UnhookWndProc(HWND hWnd);

    void Reset();

    bool IsPressed(int vk) const { return keyStates[vk].down && !disableKeyboard; }
    bool IsJustPressed(int vk) const { return keyStates[vk].pressed && !disableKeyboard; }
    bool IsReleased(int vk) const { return keyStates[vk].released && !disableKeyboard; }

    float GetScrollDelta() const { return scrollDelta; }
    int2 GetMouseDelta() const { return mouseDelta; }

    bool IsWindowJustGetFocused() const { return isWindowJustFocused; }

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

    struct NumRowKey {
        bool wasRecentlyReleased = false;
        int pressId = 0;
    } numRowKeys[NUM_KEYS_COUNT]{};
    int id = 0;

    bool isWindowFocused = true;
    bool isWindowJustFocused = true;

    static LRESULT __stdcall hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    XINPUT_STATE state;
    XINPUT_STATE prevState;
    float2 thumbLeft{};
    float2 thumbRight{};
	float leftTrigger{};
	float rightTrigger{};

public:
    bool IsGamepadPressed(WORD button) const { return (state.Gamepad.wButtons & button) != 0 && !disableController; }
    bool IsGamepadJustPressed(WORD button) const { return (state.Gamepad.wButtons & button) != 0 && (prevState.Gamepad.wButtons & button) == 0 && !disableController; }
    bool IsGamepadJustReleased(WORD button) const { return (state.Gamepad.wButtons & button) == 0 && (prevState.Gamepad.wButtons & button) != 0 && !disableController; }
    ReleasedNumkeys GetReleasedNumkeys();
    float2 GetThumbLeft() const { return disableController ? float2() : thumbLeft; }
	float2 GetThumbRight() const { return disableController ? float2() : thumbRight; }
	float GetLeftTrigger() const { return disableController ? 0 : leftTrigger; }
	float GetRightTrigger() const { return disableController ? 0 : rightTrigger; }
    void UpdateGamepad();

private:
    void UpdateKeyboard(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnWindowFocus(bool getFocused, WPARAM wParam);

    bool IsCursorVisible();
};