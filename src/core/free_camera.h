#pragma once
#include "core/game_data.h"
#include <windows.h>
#include <algorithm>

class FreeCamera {
public:
    void Update(GameData::GameRend* gameRend, float deltaTime);

    void Toggle(GameData::GameRend* rend);
    void EnableCamera(GameData::GameRend* rend, GameData::ChrIns* player);
    void DisableCamera(GameData::GameRend* rend, GameData::ChrIns* player);
    void DisableCamera();

    void SetSpeed(float newSpeed) { speed = max(newSpeed, 0.0f); }
    void SetFov(GameData::Camera *cam, float newFov) { if (cam) cam->fov = std::clamp(newFov, 0.0001f, 3.13f); }
	void SetIsSprinting(bool sprinting) { isSprinting = sprinting; }

	void AddSpeed(float delta) { SetSpeed(speed + delta); }
	void AddVelocity(const float3& delta) { velocity += delta; }
	void AddZoomVelocity(float delta) { zoomVelocity += delta; }
	void AddFov(GameData::Camera* cam, float delta) { if (cam) SetFov(cam, cam->fov + delta); }

private:
    const float SPEED_MULT = 2.5f;
    const float DEFAULT_SPEED = 10.0f;
    float speed = DEFAULT_SPEED;
    float ZOOM_SPEED = 0.5f;
    float3 velocity = float3(0);
    float zoomVelocity = 0.0f;
	bool isSprinting = false;

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
