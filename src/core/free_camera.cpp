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

void FreeCamera::Update(GameData::GameRend* gameRend, float deltaTime) {
    RestorePendingSettings();
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
    UpdateRotation(freeCamera, playerCamera, deltaTime);
    UpdateFov(freeCamera, deltaTime);

    UpdateVelocity(deltaTime);
    UpdateZoomVelocity(deltaTime);
    isSprinting = false;

    if (pathRecorder.IsRecording()) pathRecorder.RecordFrame(freeCamera);
    if (pathRecorder.IsPlaying()) pathRecorder.PlayNextFrame(freeCamera);

    frameStepper.Update();

    cameraStateManager.Update(freeCamera, float3_ref(yaw, pitch, roll), deltaTime);
}

void FreeCamera::UpdatePosition(GameData::Camera* camera, float dt) {
    const float cameraSpeed = speed * (isSprinting ? settings.speedMult : 1.0f) * dt;

    const float3& right = camera->matrix.c0.xyz();
    const float3& forward = camera->matrix.c2.xyz();
    float3 vel = right * velocity.x + forward * velocity.z + float3(0.0f, velocity.y, 0.0f);

    if (vel.lengthSquared() > 1.0f) vel = vel.normalized();

    camera->matrix.position() += vel * cameraSpeed;
}

void FreeCamera::UpdateRotation(GameData::Camera* freeCamera, GameData::Camera* playerCamera, float dt) {
    if (!IsUsingCustomRotation()) {
        CopyRotation(freeCamera, playerCamera);
        return;
    }

    RotationCache& c = rotationCache;

    float tiltSpeed = settings.flags.smoothCameraRotation ? settings.smoothTiltSpeed : settings.tiltSpeed;
    roll += rollVelocity * tiltSpeed * dt;
    c.roll.CacheSinCos(roll);

    const float sensitivity = settings.flags.smoothCameraRotation ? settings.smoothSensitivity * 10.0f * dt : settings.sensitivity;
    const float sens = sensitivity * ComputeZoomFactor(freeCamera->fov) * 0.001f;
    yawPitchVelocity += mouseDelta.rotate(c.roll.sin, c.roll.cos);

    yaw += yawPitchVelocity.x * sens;
    pitch += yawPitchVelocity.y * sens;
    if (settings.pitchLimit) pitch = std::clamp(pitch, -settings.pitchLimit, settings.pitchLimit);

    c.yaw.CacheSinCos(yaw);
    c.pitch.CacheSinCos(pitch);

    float3 forward = { c.yaw.sin * c.pitch.cos, -c.pitch.sin, c.yaw.cos * c.pitch.cos };
    float3 right = { c.yaw.cos, 0.0f, -c.yaw.sin };
    float3 up = float3::cross(forward, right);

    float3 rolledRight = right * c.roll.cos + up * c.roll.sin;
    float3 rolledUp = up * c.roll.cos - right * c.roll.sin;

    freeCamera->matrix.c0 = float4(rolledRight, 0.0f);
    freeCamera->matrix.c1 = float4(rolledUp, 0.0f);
    freeCamera->matrix.c2 = float4(forward, 0.0f);

    if (!settings.flags.smoothCameraRotation) {
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
    if (!settings.flags.smoothCameraMovement) {
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

void FreeCamera::ResetSettings(GameData::GameRend* gameRend) {
    GameData::Camera* freeCamera = gameRend->csDebugCam;
    GameData::Camera* playerCamera = gameRend->csPersCam1;
    if (!freeCamera || !playerCamera) return;
    ResetSettings(freeCamera, playerCamera);
}

void FreeCamera::SetSettings(const Settings& s) {
    settings = s;
    SetDefaultSpeed(settings.defaultSpeed);
    SetSpeedMult(settings.speedMult);
    SetZoomSpeed(settings.zoomSpeed);

    SetMinFov(settings.minFov);
    SetMaxFov(settings.maxFov);

    gameStateManager.SetZeroSpeedFreeze(settings.flags.zeroSpeedFreeze);
    cameraStateManager.SetInterpolationTime(settings.interpolationTime);
}

void FreeCamera::RestorePendingSettings() {
    if (!hudValueToRestore.has_value()) return;

    GameData::OptionData* optionData = GameDataManager::GetOptionData();
    if (optionData) {
        LOG_INFO("Restored hud option from %d to %d", optionData->HUD, hudValueToRestore);
        optionData->HUD = std::byte(hudValueToRestore.value());
        hudValueToRestore = std::nullopt;
    }
}

void FreeCamera::ResetSettings(GameData::Camera* freeCamera, GameData::Camera* playerCamera) {
    CopyPositionAndFov(freeCamera, playerCamera);
    CopyRotation(freeCamera, playerCamera);
    speed = settings.defaultSpeed;

    GetCameraPitchYaw(freeCamera, &pitch, &yaw);
    roll = 0;
}

float FreeCamera::ComputeZoomFactor(float fov) {
    const float min_fov = 0.00001, max_fov = 3.14f;
    float t = Math::clamp((fov - min_fov) / (max_fov - min_fov));
    return Math::quadraticEaseOut(t);
}

void FreeCamera::GetCameraPitchYaw(GameData::Camera* camera, float* _pitch, float* _yaw) {
    float3 forward = camera->matrix.c2.xyz();
    *_yaw = std::atan2(forward.x, forward.z);
    *_pitch = std::asin(-forward.y);
}

void FreeCamera::CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    std::memcpy(&toCamera->matrix.c3, &fromCamera->matrix.c3, sizeof(fromCamera->matrix.c3) + sizeof(fromCamera->fov));
}

void FreeCamera::CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    std::memcpy(&toCamera->matrix, &fromCamera->matrix, sizeof(float4) * 3);
}

void FreeCamera::Toggle(GameData::GameRend* rend) {
    rend->IsFreecamEnabled() ? DisableCamera(rend) : EnableCamera(rend);
}

void FreeCamera::ToggleFreeze() {
    Freeze(!isFrozen);
}

void FreeCamera::Freeze(bool enabled) {
    isFrozen = enabled;

    if (enabled) {
        if (settings.flags.freezeGame) {
            gameStateManager.FreezeGame(true);
        }
        else {
            if (settings.flags.freezeEntities) gameStateManager.FreezeEntities(true);
            if (settings.flags.freezePlayer) gameStateManager.FreezePlayer(true);
        }
    }
    else {
        gameStateManager.FreezeGame(false);
        gameStateManager.FreezeEntities(false);
        gameStateManager.FreezePlayer(false);
    }
}

void FreeCamera::EnableCamera(GameData::GameRend* rend) {
    GameData::Camera* freeCamera = rend->csDebugCam;
    GameData::Camera* playerCamera = rend->csPersCam1;
    if (!freeCamera || !playerCamera) return;

    if (settings.flags.hideHud) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            savedHudOption = optionData->HUD;
            optionData->HUD = std::byte(0);
            isHudHidden = true;
        }
    }

    SettingsBackup::SetEnabled(1);
    SettingsBackup::SaveHudValue((int)savedHudOption);

    if (settings.flags.resetCameraSettings || isFirstEnabled) {
        ResetSettings(freeCamera, playerCamera);
        isFirstEnabled = false;
    }

    if (IsUsingCustomRotation()) {
        GetCameraPitchYaw(freeCamera, &pitch, &yaw);
    }

    Freeze(true);

    frameStepper.Reset();

    velocity = float3(0);
    yawPitchVelocity = float2(0);
    zoomVelocity = 0.0f;
    rollVelocity = 0.0;

    rend->EnableFreecam(settings.flags.disablePlayerControls);
    isEnabled = true;
    LOG_INFO("Free camera enabled");
}

void FreeCamera::DisableCamera(GameData::GameRend* rend) {
    rend->DisableFreecam();

    if (isHudHidden) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            optionData->HUD = savedHudOption;
            isHudHidden = false;
        }
    }

    SettingsBackup::SetEnabled(0);

    Freeze(false);

    if (pathRecorder.IsRecording()) pathRecorder.EndRecord();
    if (pathRecorder.IsPlaying()) pathRecorder.EndPlay();

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