#pragma once
#include "core/game_data.h"
#include <windows.h>

class FreeCamera {
public:
    void Update(GameData::GameRend* gameRend, float deltaTime);

private:
    struct KeyState {
        bool wasPressed = false;

        bool IsPressedOnce(int vkKey) {
            bool isPressed = GetAsyncKeyState(vkKey) & 0x8000;
            bool trigger = isPressed && !wasPressed;
            wasPressed = isPressed;
            return trigger;
        }
    } f1Key;

    void HandleMovement(GameData::Camera* camera, float deltaTime);

    void CopyPositionAndFov(GameData::GameRend* mgr);

    void CopyRotation(GameData::Camera* fromCamera, GameData::Camera* toCamera);

    void Toggle(GameData::GameRend* mgr);
};
