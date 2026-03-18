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
    void ResetSettings(GameData::GameRend* gameRend);

	void SetSpeedMult(float newMult) { speedMult = max(newMult, 0.0f); }
    void SetDefaultSpeed(float newSpeed) { defaultSpeed = max(newSpeed, 0.0f); }
    void SetSpeed(float newSpeed) { speed = max(newSpeed, 0.0f); }
	void SetZoomSpeed(float newZoomSpeed) { zoomSpeed = max(newZoomSpeed, 0.0f); }
    void SetFov(GameData::Camera *cam, float newFov) { if (cam) cam->fov = std::clamp(newFov, minFov, maxFov); }
	void SetMinFov(float newMinFov) { minFov = std::clamp(newMinFov, MIN_FOV, MAX_FOV); }
	void SetMaxFov(float newMaxFov) { maxFov = std::clamp(newMaxFov, MIN_FOV, MAX_FOV); }
	void SetIsSprinting(bool enabled) { isSprinting = enabled; }
    void SetHideHud(bool enabled) { isHideHud = enabled; }
    void SetFreezeGame(bool enabled) { isFreezeGame = enabled; }
    void SetFreezeEntities(bool enabled) { isFreezeEntities = enabled; }
	void SetFreezePlayer(bool enabled) { isFreezePlayer = enabled; }
	void SetDisablePlayerControls(bool enabled) { isDisablePlayerControls = enabled; }
    void SetSmoothCamera(bool enabled) { isSmoothCamera = enabled; }
    void SetZeroSpeedFreeze(bool enabled) { isZeroSpeedFreeze = enabled; }
    void SetResetCameraSettings(bool enabled) { isResetCameraSettings = enabled; }
    void SetAlwaysUseCustomRotation(bool enabled) { isAlwaysUseCustomRotation = enabled; }

    void SetSensitivity(float sens) { mouseSensitivity = sens; }
    void SetPitchLimit(float limit) { pitchLimit = limit; }
    void SetMouseDeltaX(int x) { mouseDeltaX = x; }
    void SetMouseDeltaY(int y) { mouseDeltaY = y; }
    void SetTiltXVeloctiy(float vel) { tiltXVelocity = vel; }
    void SetTiltSpeed(float speed) { tiltSpeed = speed; }

	void AddSpeed(float delta) { SetSpeed(speed + delta); }
	void AddVelocity(const float3& delta) { velocity += delta; }
	void AddZoomVelocity(float delta) { zoomVelocity += delta; }
	void AddFov(GameData::Camera* cam, float delta) { if (cam) SetFov(cam, cam->fov + delta); }

private:
    float speedMult = 2.5f;
    float defaultSpeed = 10.0f;
    float speed = defaultSpeed;
    float zoomSpeed = 0.7f;
    const float MIN_FOV = 0.000126f, MAX_FOV = 3.13f;
    float minFov = MIN_FOV, maxFov = MAX_FOV;
    float pitchLimit = 1.55f;
    
    float3 velocity = float3(0);
    float zoomVelocity = 0.0f;
	bool isSprinting = false;
    std::byte savedHudOption = std::byte(2);

	bool isHideHud = true;
    bool isFreezeGame = false;
	bool isFreezeEntities = true;
	bool isFreezePlayer = true;
	bool isDisablePlayerControls = true;
	bool isSmoothCamera = true;
    bool isZeroSpeedFreeze = false;
    bool isResetCameraSettings = false;
    bool isAlwaysUseCustomRotation = false;

    bool isEnabled = false;
    bool isFristEnabled = true;

    float tiltSpeed = 1.0f;
    float tiltXVelocity = 0.0f;

    float mouseSensitivity = 0.001f;
	float yaw = 0.0f;
	float pitch = 0.0f;
    float mouseDeltaX = 0;
    float mouseDeltaY = 0;

    void UpdatePosition(GameData::Camera* camera, float dt);
    void UpdateRotation(GameData::Camera* freeCamera, GameData::Camera* playerCamera, float dt);
    void UpdateFov(GameData::Camera* camera, float dt);
	void UpdateVelocity(float dt);
	void UpdateZoomVelocity(float dt);

    void ResetSettings(GameData::Camera* freeCamera, GameData::Camera* playerCamera);

    void CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    void CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    float ComputeZoomFactor(float fov);

    bool IsUsingCustomRotation() const { return isFreezeGame || !isResetCameraSettings || isAlwaysUseCustomRotation; }

	void FreezeEntity(GameData::ChrIns* entity, bool enabled);
    void FreezePlayer(bool enabled);
    void FreezeEntities(bool enabled);
};
