#include "free_camera.h"
#include "core/game_data_manager.h"
#include "utils/types.h"
#include <algorithm>

void FreeCamera::Update(float deltaTime) {
    GameData::FieldArea* fieldArea = GameDataManager::GetFieldArea();
    if (!fieldArea) {
        Sleep(1000);
        return;
    }

    GameData::CameraManager* cameraMgr = fieldArea->cameraMgr;

    if (f1Key.IsPressedOnce(VK_F1)) {
        Toggle(cameraMgr);
    }

    if (!cameraMgr->isFreecamEnabled()) {
        return;
    }

    GameData::Camera* csDebugCam = cameraMgr->csDebugCam;

    CopyRotation(csDebugCam, cameraMgr->csPersCam1);
    HandleMovement(csDebugCam, deltaTime);
}

void FreeCamera::HandleMovement(GameData::Camera* camera, float deltaTime) {
    bool isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

    const float baseSpeed = 10.0f;
    const float shiftMult = 2.0f;
    const float speed = baseSpeed * deltaTime * (isShiftPressed ? shiftMult : 1);

    float3 velocity(0);
    if (GetAsyncKeyState('W') & 0x8000) velocity += float3::forward();
    if (GetAsyncKeyState('S') & 0x8000) velocity += float3::back();
    if (GetAsyncKeyState('A') & 0x8000) velocity += float3::left();
    if (GetAsyncKeyState('D') & 0x8000) velocity += float3::right();
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) velocity += float3::up();
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) velocity += float3::down();

    if (velocity.lengthSquared()) {
        velocity = velocity.normalized() * speed;
        camera->matrix.position() += camera->matrix.transform_vector(camera->matrix, velocity);
    }

    const float zoomSpeed = 0.3f * deltaTime;
    if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000) camera->fov -= zoomSpeed;
    if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000) camera->fov += zoomSpeed;

    camera->fov = std::clamp(camera->fov, 0.0001f, 3.13f);
}

void FreeCamera::CopyPositionAndFov(GameData::CameraManager* mgr) {
    if (!mgr || !mgr->csPersCam1 || !mgr->csDebugCam) return;

    std::memcpy(&mgr->csDebugCam->matrix.c3.x, &mgr->csPersCam1->matrix.c3.x, 20);
}

void FreeCamera::CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    if (!fromCamera || !toCamera) return;

    // std::memcpy(&toCamera->matrix.c0.x, &fromCamera->matrix.c0.x, 12);
    // std::memcpy(&toCamera->matrix.c1.x, &fromCamera->matrix.c1.x, 12);
    // std::memcpy(&toCamera->matrix.c2.x, &fromCamera->matrix.c2.x, 12);

    std::memcpy(&toCamera->matrix, &fromCamera->matrix, 16 * 3);
}

void FreeCamera::Toggle(GameData::CameraManager* mgr) {
    if (!mgr) return;

    GameData::WorldChrMan* worldCharMan = GameDataManager::GetWorldChrMan();
    if (!worldCharMan) return;

    GameData::Players* players = worldCharMan->players;
    if (!players) return;

    GameData::ChrIns* player = players->player0;
    if (!player) return;

    if (mgr->isFreecamEnabled()) {
        mgr->freeCameraMode = GameData::FreecamMode::Disabled;
        player->noMove = false;
        return;
    }

    player->noMove = true;

    CopyPositionAndFov(mgr);
    CopyRotation(mgr->csDebugCam, mgr->csPersCam1);

    mgr->freeCameraMode = GameData::FreecamMode::EnabledUpdating;
}