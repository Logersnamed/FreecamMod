#include "core/free_camera.h"

#include <algorithm>

#include "core/game_data_manager.h"
#include "utils/types.h"
#include "utils/debug.h"
#include "utils/math.h"

void FreeCamera::Update(GameData::GameRend* gameRend, float deltaTime) {
    if (!gameRend) {
        Logger::Warn("GameRend is null in FreeCamera::Update");
        return;
    }
    if (!gameRend->isFreecamEnabled()) return;

    GameData::Camera* csDebugCam = gameRend->csDebugCam;
    GameData::Camera* csPersCam1 = gameRend->csPersCam1;

    if (!csDebugCam || !csPersCam1) return;

    CopyRotation(csDebugCam, csPersCam1);
    HandleMovement(csDebugCam, deltaTime);
}

void FreeCamera::HandleMovement(GameData::Camera* camera, float deltaTime) {
    const float cameraSpeed = speed * deltaTime * (isSprinting ? speedMult : 1.0f);

    const float3& right = camera->matrix.c0.xyz();
    const float3& forward = camera->matrix.c2.xyz();
    float3 vel = right * velocity.x + forward * velocity.z;
    vel.y += velocity.y;
    if (vel.lengthSquared() > 1.0f) vel = vel.normalized();
    camera->matrix.position() += vel * cameraSpeed;

    float maxZoomStep = 0.05f;
    float zoom = zoomVelocity * zoomSpeed * deltaTime * ComputeZoomFactor(camera->fov);
    AddFov(camera, std::clamp(zoom, -maxZoomStep, maxZoomStep));

    const float zoomFadeSpeed = 16.0f;
    zoomVelocity *= 1.0f / (1.0f + zoomFadeSpeed * deltaTime);
    if (std::abs(zoomVelocity) < 0.001f) zoomVelocity = 0;

    if (isSmoothCamera) {
        const float velocityFadeSpeed = 14.0f;
        velocity *= fastEase(deltaTime * velocityFadeSpeed);
        if (velocity.lengthSquared() < 0.001f) velocity = float3(0);
    }
    else {
		velocity = float3(0);
    }

	isSprinting = false;
}

float FreeCamera::ComputeZoomFactor(float fov) {
    float t = std::clamp((fov - minFov) / (maxFov - minFov), 0.0f, 1.0f);
    return quadraticEaseOut(t);
}

void FreeCamera::CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    std::memcpy(&toCamera->matrix.c3, &fromCamera->matrix.c3, sizeof(fromCamera->matrix.c3) + sizeof(fromCamera->fov));
}

void FreeCamera::CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    std::memcpy(&toCamera->matrix, &fromCamera->matrix, sizeof(float4) * 3);
}

void FreeCamera::Toggle(GameData::GameRend* rend) {
    if (!rend) return;

	rend->isFreecamEnabled() ? DisableCamera(rend) : EnableCamera(rend);
}

void FreeCamera::EnableCamera(GameData::GameRend* rend) {
    GameData::Camera* csDebugCam = rend->csDebugCam;
    GameData::Camera* csPersCam1 = rend->csPersCam1;

    if (isHideHud) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            savedHudOption = optionData->HUD;
            optionData->HUD = std::byte(0);
        }
    }

    if (isFreezeEntities) FreezeEntities(true);
    FreezePlayer(true);

    speed = defaultSpeed;
    velocity = float3(0);
    zoomVelocity = 0;

    CopyPositionAndFov(csDebugCam, csPersCam1);
    CopyRotation(csDebugCam, csPersCam1);

    rend->freeCameraMode = GameData::FreecamMode::EnabledUpdating;
	Logger::Info("Free camera enabled");
}

void FreeCamera::DisableCamera(GameData::GameRend* rend) {
    rend->freeCameraMode = GameData::FreecamMode::Disabled;

    if (isHideHud) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            optionData->HUD = savedHudOption;
        }
    }

    if (isFreezeEntities) FreezeEntities(false);
	FreezePlayer(false);

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

void FreeCamera::FreezePlayer(bool enabled) {
    GameData::ChrIns* player = GameDataManager::GetPlayer();
    if (player) {
        player->flags2.noUpdate = enabled;
        player->flags1.noHit = enabled;
    }
}

void FreeCamera::FreezeEntities(bool enabled) {
    GameData::WorldChrMan* world = GameDataManager::GetWorldChrMan();
    if (world) {
        size_t length = world->GetEntityListLenght();
		Logger::Info("Set noUpdate = %d to %zu entities", enabled, length);
        for (size_t i = 0; i < length; i++) {
            GameData::ChrIns* chr = world->begin[i];
            if (!chr) continue;

            chr->flags2.noUpdate = enabled;
            chr->flags1.noHit = enabled;
        }
    }
}