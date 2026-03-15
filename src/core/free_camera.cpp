#include "core/free_camera.h"

#include <algorithm>

#include "core/game_data_manager.h"
#include "utils/types.h"
#include "utils/debug.h"
#include "utils/math.h"

void FreeCamera::Update(GameData::GameRend* gameRend, float deltaTime) {
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

    CopyRotation(freeCamera, playerCamera);
	UpdatePosition(freeCamera, deltaTime);
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
        }
    }

    if (isFreezeEntities) FreezeEntities(true);
    if (isFreezePlayer) FreezePlayer(true);

    speed = defaultSpeed;
    velocity = float3(0);
    zoomVelocity = 0.0f;

    CopyPositionAndFov(freeCamera, playerCamera);
    CopyRotation(freeCamera, playerCamera);

	rend->EnableFreecam(isDisablePlayerControls);
    isEnabled = true;
	Logger::Info("Free camera enabled");
}

void FreeCamera::DisableCamera(GameData::GameRend* rend) {
	rend->DisableFreecam();

    if (isHideHud) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            optionData->HUD = savedHudOption;
        }
    }

    if (isFreezeEntities) FreezeEntities(false);
    if (isFreezePlayer) FreezePlayer(false);

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
    if (isOnlyFreezeAnim) {
        entity->chrModules->chrBehavior->animationSpeed = !enabled;
    }
    else {
    entity->flags2.noUpdate = enabled;
    }
    entity->flags1.noHit = enabled;
}

void FreeCamera::FreezePlayer(bool enabled) {
    GameData::ChrIns* player = GameDataManager::GetPlayer();
    if (player) FreezeEntity(player, enabled);
}

void FreeCamera::FreezeEntities(bool enabled) {
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