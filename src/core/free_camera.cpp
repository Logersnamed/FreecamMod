#include "core/free_camera.h"

#include <algorithm>

#include "ModUtils.h"

#include "core/game_data_manager.h"
#include "core/settings_backup.h"
#include "utils/types.h"
#include "utils/debug.h"
#include "utils/math.h"

void FreeCamera::Update(GameData::GameRend* gameRend, float deltaTime) {
    RestorePendingSettings();
    if (!gameRend->IsFreecamEnabled()) {
        if (isEnabled) {
			Logger::Info("Freecam wasn't disabled properly, disabling now...");
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

    if (framesToStep > 0) {
        if (isGameFrozen) {
            wasGameFrozen = true;
            GameDataManager::PauseGame(false);
        }
        else if (areEntitesFrozen) {
            wasEntitiesFrozen = true;
            FreezeEntities(false);
        }
        else if (isPlayerFrozen) {
            wasPlayerFrozen = true;
            FreezePlayer(false);
        }

        if (framesToStep <= 1) {
            if (wasGameFrozen) GameDataManager::PauseGame(true);
            if (wasEntitiesFrozen) FreezeEntities(true);
            if (wasPlayerFrozen) FreezePlayer(true);
            wasGameFrozen = false;
            wasEntitiesFrozen = false;
            wasPlayerFrozen = false;
            framesToStep = 0;
        }
        else {
            --framesToStep;
        }
    }
}

void FreeCamera::UpdatePosition(GameData::Camera* camera, float dt) {
    const float cameraSpeed = speed * (isSprinting ? speedMult : 1.0f) * dt;

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

    roll += rollVelocity * tiltSpeed * dt;
    float rollSin = std::sin(roll);
    float rollCos = std::cos(roll);

    const float sens = sensitivity * ComputeZoomFactor(freeCamera->fov) * 0.001f;
    float2 rolledMouseDelta = mouseDelta.rotate(rollSin, rollCos);
    yaw   += rolledMouseDelta.x * sens;
    pitch += rolledMouseDelta.y * sens;

    if (pitchLimit) pitch = std::clamp(pitch, -pitchLimit, pitchLimit);

    float yawSin = std::sin(yaw);
    float yawCos = std::cos(yaw);
    float pitchSin = std::sin(pitch);
    float pitchCos = std::cos(pitch);

    float3 forward = { yawSin * pitchCos, -pitchSin, yawCos * pitchCos };
    float3 right = { yawCos, 0.0f, -yawSin };
    float3 up = float3::cross(forward, right);

    float3 rolledRight = right * rollCos + up * rollSin;
    float3 rolledUp = up * rollCos - right * rollSin;

    freeCamera->matrix.c0 = float4(rolledRight, 0.0f);
    freeCamera->matrix.c1 = float4(rolledUp, 0.0f);
    freeCamera->matrix.c2 = float4(forward, 0.0f);

    rollVelocity = 0;
}

void FreeCamera::UpdateFov(GameData::Camera* camera, float dt) {
    float maxZoomStep = 0.05f;
    float zoom = zoomVelocity * zoomSpeed * ComputeZoomFactor(camera->fov) * dt;
    AddFov(camera, std::clamp(zoom, -maxZoomStep, maxZoomStep));
}

void FreeCamera::UpdateVelocity(float dt) {
	if (!flags.smoothCamera) {
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

void FreeCamera::SetConfigSettings(const Settings& settings) {
    sensitivity = settings.sensitivity;
    SetDefaultSpeed(settings.defaultSpeed);
    SetSpeedMult(settings.speedMult);
    SetZoomSpeed(settings.zoomSpeed);

    SetMinFov(settings.minFov);
    SetMaxFov(settings.maxFov);
    pitchLimit = settings.pitchLimit;

    tiltSpeed = settings.tiltSpeed;

    step = settings.step;

    flags = settings.flags;
}

void FreeCamera::RestorePendingSettings() {
    if (!hudValueToRestore.has_value()) return;

    GameData::OptionData* optionData = GameDataManager::GetOptionData();
    if (optionData) {
        Logger::Info("Restored hud option from %d to %d", optionData->HUD, hudValueToRestore);
        optionData->HUD = std::byte(hudValueToRestore.value());
        hudValueToRestore = std::nullopt;
    }
}

void FreeCamera::ResetSettings(GameData::Camera* freeCamera, GameData::Camera* playerCamera) {
    CopyPositionAndFov(freeCamera, playerCamera);
    CopyRotation(freeCamera, playerCamera);
    speed = defaultSpeed;

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

void FreeCamera::EnableCamera(GameData::GameRend* rend) {
    GameData::Camera* freeCamera = rend->csDebugCam;
    GameData::Camera* playerCamera = rend->csPersCam1;
    if (!freeCamera || !playerCamera) return;

    framesToStep = 0;

    if (flags.hideHud) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            savedHudOption = optionData->HUD;
            optionData->HUD = std::byte(0);
            isHudHidden = true;
        }
    }

    SettingsBackup::SetEnabled(1);
    SettingsBackup::SaveHudValue((int)savedHudOption);

    if (flags.resetCameraSettings || isFirstEnabled) {
        ResetSettings(freeCamera, playerCamera);
        isFirstEnabled = false;
    }

    if (IsUsingCustomRotation()) {
        GetCameraPitchYaw(freeCamera, &pitch, &yaw);
    }

    if (flags.freezeGame && !isGameFrozen) {
        GameDataManager::PauseGame(true);
        isGameFrozen = true;
    }
    if (flags.freezeEntities && !flags.freezeGame) FreezeEntities(true);
    if (flags.freezePlayer && !flags.freezeGame) FreezePlayer(true);

    velocity = float3(0);
    zoomVelocity = 0.0f;

	rend->EnableFreecam(flags.disablePlayerControls);
    isEnabled = true;
	Logger::Info("Free camera enabled");
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

    if (isGameFrozen) {
        GameDataManager::PauseGame(false);
        isGameFrozen = false;
    }
    FreezeEntities(false);
    FreezePlayer(false);

    if (pathRecorder.IsRecording()) pathRecorder.EndRecord();
    if (pathRecorder.IsPlaying()) pathRecorder.EndPlay();

    isEnabled = false;
	Logger::Info("Free camera disabled");
}

void FreeCamera::DisableCamera() {
	GameData::FieldArea* fieldArea = GameDataManager::GetFieldArea();
    if (!fieldArea) {
		Logger::Warn("FieldArea is null in FreeCamera::DisableCamera. Nothing to disable.");
        return;
    }

    DisableCamera(fieldArea->gameRend);
}

void FreeCamera::FreezeEntity(GameData::ChrIns* entity, bool enabled) {
    if (flags.zeroSpeedFreeze) {
        entity->chrModules->chrBehavior->animationSpeed = !enabled;
    }
    else {
        entity->flags2.noUpdate = enabled;
    }
    entity->flags1.noHit = enabled;
    entity->chrModules->chrData->flags.noDamage = enabled;
    entity->chrModules->chrData->flags.noDead = enabled;
}

void FreeCamera::FreezePlayer(bool enabled) {
    if (isPlayerFrozen == enabled) return;
    isPlayerFrozen = enabled;

    GameData::ChrIns* player = GameDataManager::GetPlayer();
    if (player) FreezeEntity(player, enabled);
}

void FreeCamera::FreezeEntities(bool enabled) {
    if (areEntitesFrozen == enabled) return;
    areEntitesFrozen = enabled;

    GameData::WorldChrMan* world = GameDataManager::GetWorldChrMan();
    if (!world) return;

    GameData::Players* players = world->players;
    if (!players) return;

    if (!players->IsPlayerAlone()) {
        Logger::Info("Freezing entities in online is disabled");
        return;
    }

	GameData::ChrIns* player = players->player0;

    const size_t length = world->GetEntityListLenght();
	Logger::Info("Set FreezeEntity = %d to %zu entities", enabled, length);
    for (size_t i = 0; i < length; ++i) {
        GameData::ChrIns* entity = world->begin[i];
        if (entity && entity != player) FreezeEntity(entity, enabled);
    }
}