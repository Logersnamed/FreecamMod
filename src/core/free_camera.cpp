#include "core/free_camera.h"
#include "core/game_data_manager.h"
#include "utils/types.h"
#include "universal_wndproc_hook.h"
#include <algorithm>

void FreeCamera::Update(GameData::GameRend* gameRend, float deltaTime) {
    if (!gameRend) return;
    if (f1Key.IsPressedOnce(VK_F1)) Toggle(gameRend);
    if (!gameRend->isFreecamEnabled()) return;

    GameData::Camera* csDebugCam = gameRend->csDebugCam;

    CopyRotation(csDebugCam, gameRend->csPersCam1);
    HandleMovement(csDebugCam, deltaTime);
}

void FreeCamera::HandleMovement(GameData::Camera* camera, float deltaTime) {
	if (!camera) return;
    const float cameraSpeed = speed * deltaTime * (GetAsyncKeyState(VK_LBUTTON) ? SPEED_MULT : 1);

    float3 velocity(0);
    if (GetAsyncKeyState('W') & 0x8000) velocity += float3::forward();
    if (GetAsyncKeyState('S') & 0x8000) velocity += float3::back();
    if (GetAsyncKeyState('A') & 0x8000) velocity += float3::left();
    if (GetAsyncKeyState('D') & 0x8000) velocity += float3::right();

    if (velocity.lengthSquared()) {
        velocity = camera->matrix.transform_vector(camera->matrix, velocity);
    }

    if (GetAsyncKeyState(VK_SPACE) & 0x8000) velocity += float3::up();
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) velocity += float3::down();
    camera->matrix.position() += velocity.normalized() * cameraSpeed;

    // TODO: make zoom non-linear
    const float zoomSpeed = 0.3f * deltaTime;
    if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000) camera->fov -= zoomSpeed;
    if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000) camera->fov += zoomSpeed;

    if (UWPH::ScrollDelta != 0) {
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
            camera->fov -= UWPH::ScrollDelta * 0.03;
            camera->fov = std::clamp(camera->fov, 0.0001f, 3.13f);
        }
        else {
            speed = max(speed + UWPH::ScrollDelta, 0.0f);
        }
        UWPH::ScrollDelta = 0;
    }
}

void FreeCamera::CopyPositionAndFov(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    if (!toCamera || !fromCamera) return;
    std::memcpy(&toCamera->matrix.c3, &fromCamera->matrix.c3, 20);
}

void FreeCamera::CopyRotation(GameData::Camera* toCamera, GameData::Camera* fromCamera) {
    if (!fromCamera || !toCamera) return;
    std::memcpy(&toCamera->matrix, &fromCamera->matrix, 16 * 3);
}

void FreeCamera::Toggle(GameData::GameRend* rend) {
    if (!rend) return;
    GameData::ChrIns* player = GameDataManager::GetPlayer();
    if (!player) return;

	rend->isFreecamEnabled() ? DisableCamera(rend, player) : EnableCamera(rend, player);
}

void FreeCamera::EnableCamera(GameData::GameRend* rend, GameData::ChrIns* player) {
    if (!rend || !player) return;
    player->noMove = true;
    speed = DEFAULT_SPEED;

    CopyPositionAndFov(rend->csDebugCam, rend->csPersCam1);
    CopyRotation(rend->csDebugCam, rend->csPersCam1);

    rend->freeCameraMode = GameData::FreecamMode::EnabledUpdating;
}

void FreeCamera::DisableCamera(GameData::GameRend* rend, GameData::ChrIns* player) {
	if (!rend || !player) return;
    rend->freeCameraMode = GameData::FreecamMode::Disabled;
    player->noMove = false;
}
