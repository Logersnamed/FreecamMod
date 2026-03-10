#pragma once
#include <windows.h>
#include <algorithm>
#include <cstddef>

#include "core/game_data/game_data.h"

class FreeCamera {
public:
    void Update(GameData::GameRend* gameRend, float deltaTime);

    void Toggle(GameData::GameRend* rend);
    void EnableCamera(GameData::GameRend* rend);
    void DisableCamera(GameData::GameRend* rend);
    void DisableCamera();

	void SetSpeedMult(float newMult) { speedMult = max(newMult, 0.0f); }
    void SetDefaultSpeed(float newSpeed) { defaultSpeed = max(newSpeed, 0.0f); }
    void SetSpeed(float newSpeed) { speed = max(newSpeed, 0.0f); }
	void SetZoomSpeed(float newZoomSpeed) { zoomSpeed = max(newZoomSpeed, 0.0f); }
    void SetFov(GameData::Camera *cam, float newFov) { if (cam) cam->fov = std::clamp(newFov, minFov, maxFov); }
	void SetMinFov(float newMinFov) { minFov = std::clamp(newMinFov, MIN_FOV, MAX_FOV); }
	void SetMaxFov(float newMaxFov) { maxFov = std::clamp(newMaxFov, MIN_FOV, MAX_FOV); }
	void SetIsSprinting(bool enabled) { isSprinting = enabled; }
    void SetHideHud(bool enabled) { isHideHud = enabled; }
    void SetFreezeEntities(bool enabled) { isFreezeEntities = enabled; }
    void SetSmoothCamera(bool enabled) { isSmoothCamera = enabled; }

	void AddSpeed(float delta) { SetSpeed(speed + delta); }
	void AddVelocity(const float3& delta) { velocity += delta; }
	void AddZoomVelocity(float delta) { zoomVelocity += delta; }
	void AddFov(GameData::Camera* cam, float delta) { if (cam) SetFov(cam, cam->fov + delta); }

private:
    float speedMult = 2.5f;
    float defaultSpeed = 10.0f;
    float speed = defaultSpeed;
    float zoomSpeed = 0.7f;
    float minFov = MIN_FOV, maxFov = MAX_FOV;
	const float MIN_FOV = 0.000126f, MAX_FOV = 3.13f;
    
    float3 velocity = float3(0);
    float zoomVelocity = 0.0f;
	bool isSprinting = false;
    std::byte savedHudOption = std::byte(2);
	bool isHideHud = true;
	bool isFreezeEntities = true;
	bool isSmoothCamera = true;

    bool isEnabled = false;

    void HandleMovement(GameData::Camera* camera, float deltaTime);
    void CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    void CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    float ComputeZoomFactor(float fov);

    void FreezePlayer(bool enabled);
    void FreezeEntities(bool enabled);
};
