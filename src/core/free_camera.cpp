#include "core/free_camera.h"

#include <algorithm>

#include "core/game_data_manager.h"
#include "utils/types.h"
#include "utils/debug.h"

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
    const float cameraSpeed = speed * deltaTime * (isSprinting ? speedMult : 1);

    float3 vel(velocity.x, 0, velocity.z);
    if (vel.lengthSquared()) {
        vel = camera->matrix.transform_vector(camera->matrix, vel);
    }
    vel.y += velocity.y;
    camera->matrix.position() += vel.normalized() * cameraSpeed;

    float maxZoomStep = 0.05f;
    float zoom = zoomVelocity * zoomSpeed * deltaTime * ComputeZoomFactor(camera->fov);
    AddFov(camera, std::clamp(zoom, -maxZoomStep, maxZoomStep));

    const float zoomFade = 16.0f;
    zoomVelocity *= std::exp(-zoomFade * deltaTime);
    if (std::abs(zoomVelocity) < 0.001f) zoomVelocity = 0;

	velocity = float3(0);
	isSprinting = false;
}

float FreeCamera::ComputeZoomFactor(float fov) {
    const float minFov = 0.0f;
    const float maxFov = 3.14f;

    float t = (fov - minFov) / (maxFov - minFov);
    t = std::clamp(t, 0.0f, 1.0f);

    return 1.0f - std::pow(2.0f * t - 1.0f, 2.0f);
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

    if (autoDisableHud) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            savedHudOption = optionData->HUD;
            optionData->HUD = std::byte(0);
        }
    }

    if (disableEnemiesMovement) DisableEnemiesMovement(true);
    FreezePlayer(true);

    speed = defaultSpeed;

    CopyPositionAndFov(csDebugCam, csPersCam1);
    CopyRotation(csDebugCam, csPersCam1);

    rend->freeCameraMode = GameData::FreecamMode::EnabledUpdating;
	Logger::Info("Free camera enabled");
}

void FreeCamera::DisableCamera(GameData::GameRend* rend) {
    rend->freeCameraMode = GameData::FreecamMode::Disabled;

    if (autoDisableHud) {
        GameData::OptionData* optionData = GameDataManager::GetOptionData();
        if (optionData) {
            optionData->HUD = savedHudOption;
        }
    }

    if (disableEnemiesMovement) DisableEnemiesMovement(false);
	FreezePlayer(false);

	Logger::Info("Free camera disabled");
}

void FreeCamera::DisableCamera() {
	GameData::FieldArea* fieldArea = GameDataManager::GetFieldArea();
    if (!fieldArea) {
		Logger::Warn("FieldArea is null in FreeCamera::DisableCamera. Nothing to disable tho");
        return;
    }

    DisableCamera(fieldArea->gameRend);
}

void FreeCamera::FreezePlayer(bool enabled) {
    GameData::ChrIns* player = GameDataManager::GetPlayer();
    if (player) {
        player->noMove = enabled;
    }
}

void FreeCamera::DisableEnemiesMovement(bool enabled) {
    std::byte* allNoAttack = GameDataManager::GetChrDbgFlag(GameData::ChrDbgFlags::allNoAttack);
    std::byte* allNoUpdateAI = GameDataManager::GetChrDbgFlag(GameData::ChrDbgFlags::allNoUpdateAI);
    if (allNoAttack && allNoUpdateAI) {
        *allNoAttack = std::byte(enabled);
        *allNoUpdateAI = std::byte(enabled);
    }
}
