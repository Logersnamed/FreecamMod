#include "core/free_camera.h"
#include "core/game_data_manager.h"
#include "utils/types.h"
#include "utils/debug.h"
#include <algorithm>

void FreeCamera::Update(GameData::GameRend* gameRend, float deltaTime) {
    if (!gameRend) {
        Logger::Warn("GameRend is null in FreeCamera::Update");
        return;
    }
    if (!gameRend->isFreecamEnabled()) return;

    GameData::Camera* csDebugCam = gameRend->csDebugCam;

    CopyRotation(csDebugCam, gameRend->csPersCam1);
    HandleMovement(csDebugCam, deltaTime);
}

void FreeCamera::HandleMovement(GameData::Camera* camera, float deltaTime) {
    if (!camera) {
		Logger::Warn("Camera is null in FreeCamera::HandleMovement");
        return;
    }
    const float cameraSpeed = speed * deltaTime * (isSprinting ? speedMult : 1);

    float3 vel(velocity.x, 0, velocity.z);
    if (vel.lengthSquared()) {
        vel = camera->matrix.transform_vector(camera->matrix, vel);
    }
    vel.y += velocity.y;
    camera->matrix.position() += vel.normalized() * cameraSpeed;

    // TODO: make zoom non-linear
    AddFov(camera, zoomVelocity * zoomSpeed * deltaTime);

	velocity = 0;
    zoomVelocity = 0;
	isSprinting = false;
}

void FreeCamera::CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    if (!toCamera || !fromCamera) { 
		Logger::Warn("Camera is null in FreeCamera::CopyPositionAndFov");
        return; 
    }
    std::memcpy(&toCamera->matrix.c3, &fromCamera->matrix.c3, 20);
}

void FreeCamera::CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    if (!fromCamera || !toCamera) {
		Logger::Warn("Camera is null in FreeCamera::CopyRotation");
        return;
    }
    std::memcpy(&toCamera->matrix, &fromCamera->matrix, 16 * 3);
}

void FreeCamera::Toggle(GameData::GameRend* rend) {
    if (!rend) {
		Logger::Warn("GameRend is null in FreeCamera::Toggle");
        return;
    }
    GameData::ChrIns* player = GameDataManager::GetPlayer();
    if (!player) {
		Logger::Warn("Player is null in FreeCamera::Toggle");
        return;
    }

	rend->isFreecamEnabled() ? DisableCamera(rend, player) : EnableCamera(rend, player);
}

void FreeCamera::EnableCamera(GameData::GameRend* rend, GameData::ChrIns* player) {
    if (!rend || !player) {
		Logger::Warn("GameRend or Player is null in FreeCamera::EnableCamera");
        return;
    }
    player->noMove = true;
    speed = defaultSpeed;

    CopyPositionAndFov(rend->csDebugCam, rend->csPersCam1);
    CopyRotation(rend->csDebugCam, rend->csPersCam1);

    rend->freeCameraMode = GameData::FreecamMode::EnabledUpdating;
	Logger::Info("Free camera enabled");
}

void FreeCamera::DisableCamera(GameData::GameRend* rend, GameData::ChrIns* player) {
    if (!rend || !player) {
		Logger::Warn("GameRend or Player is null in FreeCamera::DisableCamera");
        return;
    }
    rend->freeCameraMode = GameData::FreecamMode::Disabled;
    player->noMove = false;
	Logger::Info("Free camera disabled");
}

void FreeCamera::DisableCamera() {
	GameData::FieldArea* fieldArea = GameDataManager::GetFieldArea();
    if (!fieldArea) {
		Logger::Warn("FieldArea is null in FreeCamera::DisableCamera. Nothing to disable tho");
        return;
    }

    DisableCamera(fieldArea->gameRend, GameDataManager::GetPlayer());
}
