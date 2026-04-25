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
#include "utils/bitflags.h"

enum class FreecamFlag : uint16_t {
    freezeGame              = 1 << 0,
    freezeEntities          = 1 << 1,
    freezePlayer            = 1 << 2,
    disablePlayerControls   = 1 << 3,
    resetCameraView         = 1 << 4,
    hideHud                 = 1 << 5,
    disableAA               = 1 << 6,
    disableMotionBlur       = 1 << 7,
    smoothCameraMovement    = 1 << 8,
    smoothCameraRotation    = 1 << 9,
    zeroSpeedFreeze         = 1 << 10
};

class FreeCamera {
    using enum FreecamFlag;

public:
    struct Settings {
		Flags<FreecamFlag> flags {
			freezeGame | freezeEntities | freezePlayer | disablePlayerControls | 
            resetCameraView | hideHud | smoothCameraMovement
        };

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

        Settings operator=(Settings other) {
            other.speedMult = max(other.speedMult, 0.0f);
            other.defaultSpeed = max(other.defaultSpeed, 0.0f);
            other.zoomSpeed = max(other.zoomSpeed, 0.0f);

            other.minFov = std::clamp(other.minFov, MIN_FOV, MAX_FOV);
            other.maxFov = std::clamp(other.maxFov, MIN_FOV, MAX_FOV);
            return other;
        }
    };

    FreeCamera() : frameStepper(gameStateManager) {}

    bool Initialize();

    void OnConfigReload();
    void Update(GameData::GameRend* gameRend, float deltaTime);

    void Toggle(GameData::GameRend* rend);
    void EnableCamera(GameData::GameRend* rend);
    void DisableCamera(GameData::GameRend* rend);
    void DisableCamera();

    void ToggleFreeze();
    void ResetCameraView(GameData::GameRend* gameRend);
    void SetSettings(const Settings& s) { settings = s; };


    void SetMouseDelta(int2 delta) { mouseDelta = delta; }
    void SetIsSprinting(bool enabled) { isSprinting = enabled; }

    void AddSpeed(float deltaSpeed) { SetSpeed(speed + deltaSpeed * (0.05f * speed + 0.01f)); }
    void AddVelocity(const float3& deltaVel) { velocity += deltaVel; }
    void AddRollVelocity(float deltaRoll) { rollVelocity += deltaRoll; }
    void AddZoomVelocity(float deltaZoom) { zoomVelocity += deltaZoom; }
    void AddFov(GameData::Camera* cam, float deltaFov) { SetFov(cam, cam->fov + deltaFov); }

    bool IsEnabled() const { return isEnabled; }
    float3 GetYawPitchRoll() const { return { yaw, pitch, roll }; }

    PathRecorder& GetPathRecorder() { return pathRecorder; }
    CameraStateManager& GetCameraStateManager() { return cameraStateManager; }
    void StepFrames() { frameStepper.StepFrames(settings.step); }

private:
    bool isEnabled = false;
    bool isFirstEnable = true;
    bool isSprinting = false;
    bool isFrozen = false;

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

    static constexpr float MIN_FOV = 0.000126f;
    static constexpr float MAX_FOV = 3.13f;

    void UpdatePosition(GameData::Camera* camera, float dt);
    void UpdateRotation(GameData::Camera* freeCamera, float dt);
    void UpdateFov(GameData::Camera* camera, float dt);
    void UpdateVelocity(float dt);
    void UpdateZoomVelocity(float dt);

    void ApplyFreezeState(bool enabled);
    void ApplyGameOptions(bool enabled);
    void RestorePendingOptions();
    void ResetCameraView(GameData::Camera* freeCamera, GameData::Camera* playerCamera);

    void CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    void CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    float ComputeZoomFactor(float fov);

    bool flagged(FreecamFlag flag) const { return settings.flags.get(flag); }

    void GetCameraPitchYaw(GameData::Camera* camera, float* _pitch, float* _yaw);

    void SetSpeed(float newSpeed) { speed = max(newSpeed, 0.0f); }
    void SetFov(GameData::Camera* cam, float newFov) { cam->fov = std::clamp(newFov, settings.minFov, settings.maxFov); }

    struct RotationCache {
        struct AngleCached {
            float angle = 0;
            float sin = 0;
            float cos = 1;

            void CacheSinCos(float _angle) {
                if (angle != _angle) {
                    angle = _angle;
                    sin = std::sinf(angle);
                    cos = std::cosf(angle);
                }
            }
        } yaw, pitch, roll;
    } rotationCache;
};
