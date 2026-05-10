#pragma once
#include <windows.h>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <cstdint>

#include "core/config/con_var.h" 
#include "core/features/frame_stepper.h"
#include "core/features/game_state_manager.h"
#include "core/features/path_recorder.h"
#include "core/features/camera_state_manager.h"
#include "core/game_data/game_data.h"
#include "utils/types.h"
#include "utils/debug.h"

class FreeCamera {
    ConVar<bool>  freezeGame             { "freecam", "freeze_game",                  true  };
    ConVar<bool>  freezeEntities         { "freecam", "freeze_entities",              true  };
    ConVar<bool>  freezePlayer           { "freecam", "freeze_player",                true  };
    ConVar<bool>  disablePlayerControls  { "freecam", "disable_player_controls",      true  };
    ConVar<bool>  resetCameraState       { "freecam", "reset_camera_state",           true  };

    ConVar<bool>  hideHud                { "game_options", "hide_hud",                true  };
    ConVar<bool>  disableAntiAliasing    { "game_options", "disable_anti_aliasing",   false };
    ConVar<bool>  disableMotionBlur      { "game_options", "disable_motion_blur",     false };

    ConVar<float> sensitivity            { "camera_settings", "sensitivity",          1.0f  };
    ConVar<float> defaultSpeed           { "camera_settings", "default_speed",        10.0f, 0.0f };
    ConVar<float> tiltSpeed              { "camera_settings", "tilt_speed",           0.5f,  0.0f };
    ConVar<float> speedMult              { "camera_settings", "speed_multiplier",     2.5f,  0.0f };
    ConVar<float> zoomSpeed              { "camera_settings", "zoom_speed",           0.7f,  0.0f };

    ConVar<float> minFov                 { "camera_settings", "min_fov",              0.000087f, MIN_FOV, MAX_FOV };
    ConVar<float> maxFov                 { "camera_settings", "max_fov",              2.70f,     MIN_FOV, MAX_FOV };
    ConVar<float> pitchLimit             { "camera_settings", "pitch_limit",          1.55f };

    ConVar<bool>  smoothCameraMovement   { "smooth_camera_settings", "smooth_camera_movement", true  };
    ConVar<bool>  smoothCameraRotation   { "smooth_camera_settings", "smooth_camera_rotation", false };
    ConVar<float> smoothSensitivity      { "smooth_camera_settings", "sensitivity",            1.0f  };
    ConVar<float> smoothTiltSpeed        { "smooth_camera_settings", "tilt_speed",             0.05f };

public:
    FreeCamera() {}

    bool Initialize();

    void OnConfigReload();
    void Update(GameData::GameRend* gameRend, float deltaTime);

    void Toggle(GameData::GameRend* rend);
    void EnableCamera(GameData::GameRend* rend);
    void DisableCamera(GameData::GameRend* rend);
    void DisableCamera();

    void ToggleFreeze() { ApplyFreezeState(!isFrozen); }
    void ResetCameraState(GameData::GameRend* gameRend);

    void SetMouseDelta(int2 delta) { mouseDelta = delta; }
    void SetGamepadDelta(float2 delta) { gamepadDelta = delta; }
    void SetIsSprinting(bool enabled) { isSprinting = enabled; }

    void AddSpeed(float deltaSpeed) { SetSpeed(speed + deltaSpeed * (0.05f * speed + 0.01f)); }
    void AddVelocity(const float3& deltaVel) { velocity += deltaVel; }
    void AddRollVelocity(float deltaRoll) { rollVelocity += deltaRoll; }
    void AddZoomVelocity(float deltaZoom) { zoomVelocity += deltaZoom; }
    void AddFov(GameData::Camera* cam, float deltaFov) { SetFov(cam, cam->fov + deltaFov); }

    bool IsEnabled() const { return isEnabled; }
    EulerAngles& GetEuler() { return rotation; }

    PathRecorder& GetPathRecorder() { return pathRecorder; }
    CameraStateManager& GetCameraStateManager() { return cameraStateManager; }
    void StepFrames() { frameStepper.StepFrames(); }

private:
    bool isEnabled = false;
    bool isFirstEnable = true;
    bool isSprinting = false;
    bool isFrozen = false;

    GameStateManager gameStateManager{};
    FrameStepper frameStepper{ gameStateManager };
    PathRecorder pathRecorder{};
    CameraStateManager cameraStateManager{};

    float speed = 10.0f;
    float3 velocity = float3(0);
    float zoomVelocity = 0.0f;
    float2 yawPitchVelocity = 0;
    float rollVelocity = 0.0f;

    int2 mouseDelta = 0;
    float2 gamepadDelta = 0;

    EulerAngles rotation{};

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
    void ResetCameraState(GameData::Camera* freeCamera, GameData::Camera* playerCamera);

    void CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    void CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera);
    float ComputeZoomFactor(float fov);

    void SetSpeed(float newSpeed) { speed = std::max<float>(newSpeed, 0.0f); }
    void SetFov(GameData::Camera* cam, float newFov) { cam->fov = std::clamp<float>(newFov, minFov, maxFov); }

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
