#pragma once
#include "core/game_data.h"
#include <windows.h>

class FreeCamera {
public:
    void Update(GameData::GameRend* gameRend, float deltaTime);

    void Toggle(GameData::GameRend* rend);
    void EnableCamera(GameData::GameRend* rend, GameData::ChrIns* player);
    void DisableCamera(GameData::GameRend* rend, GameData::ChrIns* player);

private:
    const float SPEED_MULT = 2.0f;
    const float DEFAULT_SPEED = 10.0f;
    float speed = DEFAULT_SPEED;

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
    void CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    void CopyRotation(GameData::Camera* fromCamera, GameData::Camera* toCamera);
};
