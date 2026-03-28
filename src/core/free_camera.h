#pragma once
#include <windows.h>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <cstdint>

#include "core/features/frame_stepper.h"
#include "core/features/game_state_manager.h"
#include "core/features/path_recorder.h"
#include "core/features/camera_state_manager.h"
#include "core/game_data/game_data.h"
#include "utils/debug.h"

class FreeCamera {
public:
    struct Settings {
        struct Flags {
            bool hideHud = true;
            bool freezeGame = true;
            bool freezeEntities = true;
            bool freezePlayer = true;
            bool disablePlayerControls = true;
            bool resetCameraSettings = true;
            bool alwaysUseCustomRotation = true;

            bool smoothCameraMovement = true;
            bool smoothCameraRotation = false;

            bool zeroSpeedFreeze = false;
        } flags{};

        float sensitivity = 1.0f;
        float defaultSpeed = 10.0f;
        float tiltSpeed = 0.5f;
        float speedMult = 2.5f;
        float zoomSpeed = 0.7f;

        float minFov = 0.000087f;
        float maxFov = 2.70f;
        float pitchLimit = 1.55f;

        float smoothSensitivity = 1.0f;
        float smoothTiltSpeed = 0.05f;

        int step = 1;
        float interpolationTime = 3.0f;
    };

    FreeCamera() : frameStepper(gameStateManager) {}

    void Update(GameData::GameRend* gameRend, float deltaTime);

    void Toggle(GameData::GameRend* rend);
    void EnableCamera(GameData::GameRend* rend);
    void DisableCamera(GameData::GameRend* rend);
    void DisableCamera();
    void ResetSettings(GameData::GameRend* gameRend);
    void SetSettings(const Settings& s);

    PathRecorder& GetPathRecorder() { return pathRecorder; }
    CameraStateManager& GetCameraStateManager() { return cameraStateManager; }
    void StepFrames() { frameStepper.StepFrames(settings.step); }

    bool IsEnabled() const { return isEnabled; }
    float3 GetYawPitchRoll() const { return { yaw, pitch, roll }; }

    void SetMouseDelta(int2 delta) { mouseDelta = delta; }
    void SetIsSprinting(bool enabled) { isSprinting = enabled; }

    void AddSpeed(float deltaSpeed) { SetSpeed(speed + deltaSpeed * (0.05f * speed + 0.01f)); }
    void AddVelocity(const float3& deltaVel) { velocity += deltaVel; }
    void AddRollVelocity(float deltaRoll) { rollVelocity += deltaRoll; }
    void AddZoomVelocity(float deltaZoom) { zoomVelocity += deltaZoom; }
    void AddFov(GameData::Camera* cam, float deltaFov) { SetFov(cam, cam->fov + deltaFov); }

    void SetHudValueToRestore(std::optional<int> value) { hudValueToRestore = value; }

private:
    bool isEnabled = false;
    bool isFirstEnabled = true;

    GameStateManager gameStateManager{};
    FrameStepper frameStepper;
    PathRecorder pathRecorder{};
    CameraStateManager cameraStateManager{};

    Settings settings;

    float speed = 10.0f;
    float3 velocity = float3(0);
    float zoomVelocity = 0.0f;
    float2 yawPitchVelocity = 0;
    float rollVelocity = 0.0f;

    int2 mouseDelta = 0;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float roll = 0.0f;

    const float MIN_FOV = 0.000126f;
    const float MAX_FOV = 3.13f;
    
	bool isSprinting = false;

    bool isHudHidden = false;
    std::byte savedHudOption = std::byte(2);
    std::optional<int> hudValueToRestore = std::nullopt;

    void UpdatePosition(GameData::Camera* camera, float dt);
    void UpdateRotation(GameData::Camera* freeCamera, GameData::Camera* playerCamera, float dt);
    void UpdateFov(GameData::Camera* camera, float dt);
	void UpdateVelocity(float dt);
	void UpdateZoomVelocity(float dt);

    void RestorePendingSettings();
    void ResetSettings(GameData::Camera* freeCamera, GameData::Camera* playerCamera);

    void CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    void CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    float ComputeZoomFactor(float fov);

    bool IsUsingCustomRotation() const {
        if (!settings.flags.alwaysUseCustomRotation) return false;
        return settings.flags.freezeGame || !settings.flags.resetCameraSettings || settings.flags.alwaysUseCustomRotation;
    }

    void GetCameraPitchYaw(GameData::Camera* camera, float* _pitch, float* _yaw);

    void SetSpeed(float newSpeed) { speed = max(newSpeed, 0.0f); }
    void SetSpeedMult(float newMult) { settings.speedMult = max(newMult, 0.0f); }
    void SetDefaultSpeed(float newDefaultSpeed) { settings.defaultSpeed = max(newDefaultSpeed, 0.0f); }
    void SetZoomSpeed(float newZoomSpeed) { settings.zoomSpeed = max(newZoomSpeed, 0.0f); }
    void SetFov(GameData::Camera* cam, float newFov) { cam->fov = std::clamp(newFov, settings.minFov, settings.maxFov); }
    void SetMinFov(float newMinFov) { settings.minFov = std::clamp(newMinFov, MIN_FOV, MAX_FOV); }
    void SetMaxFov(float newMaxFov) { settings.maxFov = std::clamp(newMaxFov, MIN_FOV, MAX_FOV); }
};
