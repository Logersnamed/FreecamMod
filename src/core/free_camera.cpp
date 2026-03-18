#include "core/free_camera.h"

#include <algorithm>

#include "ModUtils.h"

#include "core/game_data_manager.h"
#include "core/settings_backup.h"
#include "utils/types.h"
#include "utils/debug.h"
#include "utils/math.h"

void FreeCamera::Update(GameData::GameRend* gameRend, float deltaTime) {
    RestoreSettings();
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

    if (!mouseDeltaX && !mouseDeltaY && !tiltXVelocity && !rollVelocity) return;

    const float sens = mouseSensitivity * freeCamera->fov;

    yaw += (mouseDeltaX + tiltXVelocity * tiltSpeed * 100.0f * dt) * sens;
    pitch += mouseDeltaY * sens;
    roll += rollVelocity * tiltSpeed * 100.0f * dt * sens;

    if (pitchLimit) pitch = std::clamp(pitch, -pitchLimit, pitchLimit);

    float cy = std::cos(yaw);
    float sy = std::sin(yaw);
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);

    float3 forward = { sy * cp, -sp, cy * cp };
    float3 right = { cy, 0.0f, -sy };
    float3 up = float3().cross(forward, right);

    float cr = std::cos(roll);
    float sr = std::sin(roll);

    float3 r = right * cr + up * sr;
    float3 u = up * cr - right * sr;

    right = r;
    up = u;

    freeCamera->matrix.c0 = float4(right, 0.0f);
    freeCamera->matrix.c1 = float4(up, 0.0f);
    freeCamera->matrix.c2 = float4(forward, 0.0f);

    tiltXVelocity = 0;
    rollVelocity = 0;
}

void FreeCamera::UpdateFov(GameData::Camera* camera, float dt) {
    float maxZoomStep = 0.05f;
    float zoom = zoomVelocity * zoomSpeed * ComputeZoomFactor(camera->fov) * dt;
    AddFov(camera, std::clamp(zoom, -maxZoomStep, maxZoomStep));
}

void FreeCamera::UpdateVelocity(float dt) {
	if (!isSmoothCamera) {
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

void FreeCamera::RestoreSettings() {
    if (initHudValue == -1) return;

    GameData::OptionData* optionData = GameDataManager::GetOptionData();
    if (optionData) {
        Logger::Info("Restored hud from %d to %d", optionData->HUD, initHudValue);
        optionData->HUD = std::byte(initHudValue);
        initHudValue = -1;
    }
}

void FreeCamera::ResetSettings(GameData::Camera* freeCamera, GameData::Camera* playerCamera) {
    CopyPositionAndFov(freeCamera, playerCamera);
    CopyRotation(freeCamera, playerCamera);
    speed = defaultSpeed;

    float3 forward = freeCamera->matrix.c2.xyz();
    yaw = std::atan2(forward.x, forward.z);
    pitch = std::asin(-forward.y);
    roll = 0;
}

float FreeCamera::ComputeZoomFactor(float fov) {
	const float min_fov = 0.00001, max_fov = 3.14f;
    float t = Math::clamp((fov - min_fov) / (max_fov - min_fov));
    return Math::quadraticEaseOut(t);
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

    if (isHideHud) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            savedHudOption = optionData->HUD;
            optionData->HUD = std::byte(0);
            isHudHidden = true;
        }
    }

    SettingsBackup::SetEnabled(1);
    SettingsBackup::SaveHudValue((int)savedHudOption);

    velocity = float3(0);
    zoomVelocity = 0.0f;


    if (isResetCameraSettings || isFristEnabled) {
        ResetSettings(freeCamera, playerCamera);
        isFristEnabled = false;
    }

    if (IsUsingCustomRotation()) {
        float3 forward = freeCamera->matrix.c2.xyz();
        yaw = std::atan2(forward.x, forward.z);
        pitch = std::asin(-forward.y);
    }

    if (isFreezeGame) GameDataManager::PauseGame(true);
    if (isFreezeEntities) FreezeEntities(true);
    if (isFreezePlayer) FreezePlayer(true);

	rend->EnableFreecam(isDisablePlayerControls);
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

    GameDataManager::PauseGame(false);
    FreezeEntities(false);
    FreezePlayer(false);

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
    if (isZeroSpeedFreeze) {
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
    if (isPlayerFreezed == enabled) return;
    isPlayerFreezed = enabled;

    GameData::ChrIns* player = GameDataManager::GetPlayer();
    if (player) FreezeEntity(player, enabled);
}

void FreeCamera::FreezeEntities(bool enabled) {
    if (areEntitesFreezed == enabled) return;
    areEntitesFreezed = enabled;

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