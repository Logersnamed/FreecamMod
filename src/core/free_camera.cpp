#include "core/free_camera.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "ModUtils.h"

#include "core/game_data/game_data.h"
#include "core/game_data_manager.h"
#include "core/settings_backup.h"
#include "utils/types.h"
#include "utils/debug.h"
#include "utils/math.h"

bool FreeCamera::Initialize() {
    auto wasFreecamEnabled = SettingsBackup::RestoreOptionValue(OptionType::Freecam);
    if (wasFreecamEnabled.has_value()) {
        if (wasFreecamEnabled.value() == 1) {
            gameStateManager.SetOptionValueToRestore(OptionType::HUD, SettingsBackup::RestoreOptionValue(OptionType::HUD));
            gameStateManager.SetOptionValueToRestore(OptionType::AA, SettingsBackup::RestoreOptionValue(OptionType::AA));
            gameStateManager.SetOptionValueToRestore(OptionType::MotionBlur, SettingsBackup::RestoreOptionValue(OptionType::MotionBlur));
        }
    }

    return true;
}

void FreeCamera::OnConfigReload() {
    if (!isEnabled) return;
    
    ApplyGameOptions(true);
    ApplyFreezeState(true);
    
    GameData::FieldArea* fieldArea = GameDataManager::FieldArea.Get();
    if (fieldArea && fieldArea->gameRend) {
        fieldArea->gameRend->EnableFreecam(flagged(disablePlayerControls));
    }

    gameStateManager.SetZeroSpeedFreeze(flagged(zeroSpeedFreeze));
    cameraStateManager.SetInterpolationTime(settings.interpolationTime);
}

void FreeCamera::Update(GameData::GameRend* gameRend, float deltaTime) {
    RestorePendingOptions();
    if (!gameRend->IsFreecamEnabled()) {
        if (isEnabled) {
            LOG_INFO("Freecam wasn't disabled properly, disabling now...");
            DisableCamera(gameRend);
        }
        return;
    }

    GameData::Camera* freeCamera = gameRend->csDebugCam;
    GameData::Camera* playerCamera = gameRend->csPersCam1;
    if (!freeCamera || !playerCamera) return;

    UpdatePosition(freeCamera, deltaTime);
    UpdateRotation(freeCamera, deltaTime);
    UpdateFov(freeCamera, deltaTime);

    UpdateVelocity(deltaTime);
    UpdateZoomVelocity(deltaTime);

    if (pathRecorder.IsRecording()) pathRecorder.RecordFrame(freeCamera);
    if (pathRecorder.IsPlaying()) pathRecorder.PlayNextFrame(freeCamera);

    frameStepper.Update();

    cameraStateManager.Update(freeCamera, rotation, deltaTime);
}

void FreeCamera::Toggle(GameData::GameRend* rend) {
    rend->IsFreecamEnabled() ? DisableCamera(rend) : EnableCamera(rend);
}

void FreeCamera::EnableCamera(GameData::GameRend* rend) {
    GameData::Camera* freeCamera = rend->csDebugCam;
    GameData::Camera* playerCamera = rend->csPersCam1;
    if (!freeCamera || !playerCamera) return;

    if (flagged(resetCameraState) || isFirstEnable) {
        ResetCameraState(freeCamera, playerCamera);
        isFirstEnable = false;
    }

    rotation = freeCamera->GetEuler();
    velocity = float3(0);
    yawPitchVelocity = float2(0);
    zoomVelocity = 0.0f;
    rollVelocity = 0.0f;

    frameStepper.Reset();

    SettingsBackup::SaveOptionValue(OptionType::Freecam, true);
    ApplyGameOptions(true);
    ApplyFreezeState(true);

    rend->EnableFreecam(flagged(disablePlayerControls));
    isEnabled = true;
    LOG_INFO("Free camera enabled");
}

void FreeCamera::DisableCamera(GameData::GameRend* rend) {
    rend->DisableFreecam();

    if (pathRecorder.IsRecording()) pathRecorder.EndRecord();
    if (pathRecorder.IsPlaying()) pathRecorder.EndPlay();

    SettingsBackup::SaveOptionValue(OptionType::Freecam, false);
    ApplyGameOptions(false);
    ApplyFreezeState(false);

    isEnabled = false;
    LOG_INFO("Free camera disabled");
}

void FreeCamera::DisableCamera() {
    GameData::FieldArea* fieldArea = GameDataManager::FieldArea.Get();
    if (!fieldArea) {
        LOG_WARN("FieldArea is null in FreeCamera::DisableCamera. Nothing to disable.");
        return;
    }

    DisableCamera(fieldArea->gameRend);
}

void FreeCamera::ResetCameraState(GameData::GameRend* gameRend) {
    GameData::Camera* freeCamera = gameRend->csDebugCam;
    GameData::Camera* playerCamera = gameRend->csPersCam1;
    if (!freeCamera || !playerCamera) return;
    ResetCameraState(freeCamera, playerCamera);
}

void FreeCamera::UpdatePosition(GameData::Camera* camera, float dt) {
    const float cameraSpeed = speed * (isSprinting ? settings.speedMult : 1.0f) * dt;

    const float3& right = camera->right();
    const float3& forward = camera->forward();
    float3 vel = right * velocity.x + forward * velocity.z + float3(0.0f, velocity.y, 0.0f);

    if (vel.lengthSquared() > 1.0f) vel = vel.normalized();

    camera->matrix.position() += vel * cameraSpeed;
}

void FreeCamera::UpdateRotation(GameData::Camera* freeCamera, float dt) {
    RotationCache& c = rotationCache;

    float tiltSpeed = flagged(smoothCameraRotation) ? settings.smoothTiltSpeed : settings.tiltSpeed;
    rotation.roll += rollVelocity * tiltSpeed * dt;
    c.roll.CacheSinCos(rotation.roll);

    const float sensitivity = flagged(smoothCameraRotation) ? settings.smoothSensitivity * 10.0f * dt : settings.sensitivity;
    const float sens = sensitivity * ComputeZoomFactor(freeCamera->fov) * 0.001f;
	const float2 devicesDelta = (float2)mouseDelta + gamepadDelta * 60.0f;
    yawPitchVelocity += devicesDelta.rotated(-c.roll.sin, c.roll.cos);

    // todo: make pitch limit depend on roll
    rotation.yaw += yawPitchVelocity.x * sens;
    rotation.pitch += yawPitchVelocity.y * sens;
    if (settings.pitchLimit) rotation.pitch = std::clamp(
        rotation.pitch, -settings.pitchLimit, settings.pitchLimit
    );

    c.yaw.CacheSinCos(rotation.yaw);
    c.pitch.CacheSinCos(rotation.pitch);

    float3 forward = { c.yaw.sin * c.pitch.cos, -c.pitch.sin, c.yaw.cos * c.pitch.cos };
    float3 right = { c.yaw.cos, 0.0f, -c.yaw.sin };
    float3 up = float3::cross(forward, right);

    float3 rolledRight = right * c.roll.cos + up * c.roll.sin;
    float3 rolledUp = up * c.roll.cos - right * c.roll.sin;

    freeCamera->matrix.c0 = float4(rolledRight, 0.0f);
    freeCamera->matrix.c1 = float4(rolledUp, 0.0f);
    freeCamera->matrix.c2 = float4(forward, 0.0f);

    if (!flagged(smoothCameraRotation)) {
        yawPitchVelocity = float2();
        rollVelocity = 0;
        return;
    }

    const float velocityFadeSpeed = 1.0f;
    float factor = Math::fastEase(velocityFadeSpeed * dt);
    yawPitchVelocity *= factor;
    rollVelocity *= factor;
    if (yawPitchVelocity.lengthSquared() < 0.001f) yawPitchVelocity = float2();
    if (std::abs(rollVelocity) < 0.001f) rollVelocity = 0;
}

void FreeCamera::UpdateFov(GameData::Camera* camera, float dt) {
    float maxZoomStep = 0.05f;
    float zoom = zoomVelocity * settings.zoomSpeed * ComputeZoomFactor(camera->fov) * dt;
    AddFov(camera, std::clamp(zoom, -maxZoomStep, maxZoomStep));
}

void FreeCamera::UpdateVelocity(float dt) {
    if (!flagged(smoothCameraMovement)) {
        velocity = float3(0);
        return;
    }

    const float velocityFadeSpeed = 14.0f;
    velocity *= Math::fastEase(velocityFadeSpeed * dt);
    if (velocity.lengthSquared() < 0.001f) velocity = float3(0);
}

void FreeCamera::UpdateZoomVelocity(float dt) {
    const float zoomFadeSpeed = 16.0f;
    zoomVelocity *= 1.0f / (1.0f + zoomFadeSpeed * dt);
    if (std::abs(zoomVelocity) < 0.001f) zoomVelocity = 0;
}

void FreeCamera::ApplyFreezeState(bool enabled) {
    isFrozen = enabled;

    bool isFreezeGame = flagged(freezeGame);
    gameStateManager.FreezeGame(enabled && isFreezeGame);
    gameStateManager.FreezeEntities(enabled && flagged(freezeEntities) && !isFreezeGame);
    gameStateManager.FreezePlayer(enabled && flagged(freezePlayer) && !isFreezeGame);
}

void FreeCamera::ApplyGameOptions(bool enabled) {
    gameStateManager.DisableOption(OptionType::HUD, enabled && flagged(hideHud));
    gameStateManager.DisableOption(OptionType::AA, enabled && flagged(disableAA));
    gameStateManager.DisableOption(OptionType::MotionBlur, enabled && flagged(disableMotionBlur));
}

void FreeCamera::RestorePendingOptions() {
    if (gameStateManager.RestoreOptions()) {
        SettingsBackup::SaveOptionValue(OptionType::Freecam, isEnabled);
    }
}

void FreeCamera::ResetCameraState(GameData::Camera* freeCamera, GameData::Camera* playerCamera) {
    CopyPositionAndFov(freeCamera, playerCamera);
    CopyRotation(freeCamera, playerCamera);
    speed = settings.defaultSpeed;
    rotation = freeCamera->GetEuler();
}

void FreeCamera::CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    std::memcpy(&toCamera->matrix.c3, &fromCamera->matrix.c3, sizeof(fromCamera->matrix.c3) + sizeof(fromCamera->fov));
}

void FreeCamera::CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    std::memcpy(&toCamera->matrix, &fromCamera->matrix, sizeof(float4) * 3);
}

float FreeCamera::ComputeZoomFactor(float fov) {
    const float min_fov = 0.00001, max_fov = 3.14f;
    float t = Math::clamp((fov - min_fov) / (max_fov - min_fov));
    return Math::quadraticEaseOut(t);
}