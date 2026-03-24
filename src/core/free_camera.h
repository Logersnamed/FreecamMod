#pragma once
#include <windows.h>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <cstdint>

#include "core/features/frame_stepper.h"
#include "core/features/freeze_controller.h"
#include "core/features/path_recorder.h"
#include "core/game_data/game_data.h"
#include "utils/debug.h"

class FreeCamera {
public:
    struct Settings {
        float sensitivity = 0.001f;
        float defaultSpeed = 10.0f;
        float speedMult = 2.5f;
        float zoomSpeed = 0.7f;

        float minFov = 0.000126f;
        float maxFov = 3.13f;
        float pitchLimit = 1.55f;

        float tiltSpeed = 1.0f;

        int step = 1;

        struct Flags {
            bool smoothCamera = true;
            bool hideHud = true;
            bool freezeGame = false;
            bool freezeEntities = true;
            bool freezePlayer = true;
            bool disablePlayerControls = true;
            bool zeroSpeedFreeze = false;
            bool resetCameraSettings = false;
            bool alwaysUseCustomRotation = false;
        } flags{};
    };

    FreeCamera() : frameStepper(freezeController) {}

    void Update(GameData::GameRend* gameRend, float deltaTime);

    void Toggle(GameData::GameRend* rend);
    void EnableCamera(GameData::GameRend* rend);
    void DisableCamera(GameData::GameRend* rend);
    void DisableCamera();
    void ResetSettings(GameData::GameRend* gameRend);
    void SetConfigSettings(const Settings& cameraSettings);

    PathRecorder& GetPathRecorder() { return pathRecorder; }
    void StepFrames() { frameStepper.StepFrames(step); }

    void SetMouseDelta(int2 delta) { mouseDelta = delta; }
    void SetRollVeloctiy(float vel) { rollVelocity = vel; }
    void SetIsSprinting(bool enabled) { isSprinting = enabled; }

	void AddSpeed(float delta) { SetSpeed(speed + delta); }
	void AddVelocity(const float3& delta) { velocity += delta; }
	void AddZoomVelocity(float delta) { zoomVelocity += delta; }
	void AddFov(GameData::Camera* cam, float delta) { if (cam) SetFov(cam, cam->fov + delta); }

    void SetHudValueToRestore(std::optional<int> value) { hudValueToRestore = value; }

private:
    bool isEnabled = false;
    bool isFirstEnabled = true;

    FreezeController freezeController{};
    FrameStepper frameStepper;
    PathRecorder pathRecorder{};
    Settings::Flags flags{};

    float speed = 10.0f;
    float3 velocity = float3(0);
    float zoomVelocity = 0.0f;
    float rollVelocity = 0.0f;

    int2 mouseDelta = 0;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float roll = 0.0f;

    float sensitivity = 0.001f;
    float zoomSpeed = 0.7f;
    float tiltSpeed = 1.0f;
    float defaultSpeed = 10.0f;
    float speedMult = 2.5f;

    const float MIN_FOV = 0.000126f;
    const float MAX_FOV = 3.13f;
    float minFov = MIN_FOV;
    float maxFov = MAX_FOV;
    float pitchLimit = 1.55f;
    
	bool isSprinting = false;

    bool isHudHidden = false;
    std::byte savedHudOption = std::byte(2);
    std::optional<int> hudValueToRestore = std::nullopt;

    int step = 1;

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
        return flags.freezeGame || !flags.resetCameraSettings || flags.alwaysUseCustomRotation; 
    }

    void GetCameraPitchYaw(GameData::Camera* camera, float* _pitch, float* _yaw);

    void SetSpeed(float newSpeed) { speed = max(newSpeed, 0.0f); }
    void SetSpeedMult(float newMult) { speedMult = max(newMult, 0.0f); }
    void SetDefaultSpeed(float newSpeed) { defaultSpeed = max(newSpeed, 0.0f); }
    void SetZoomSpeed(float newZoomSpeed) { zoomSpeed = max(newZoomSpeed, 0.0f); }
    void SetFov(GameData::Camera* cam, float newFov) { if (cam) cam->fov = std::clamp(newFov, minFov, maxFov); }
    void SetMinFov(float newMinFov) { minFov = std::clamp(newMinFov, MIN_FOV, MAX_FOV); }
    void SetMaxFov(float newMaxFov) { maxFov = std::clamp(newMaxFov, MIN_FOV, MAX_FOV); }
};
