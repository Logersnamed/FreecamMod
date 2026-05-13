#include "freecam.h"

#include <algorithm>
#include <functional>
#include <cstdint>
#include <vector>

#include "ModUtils.h"

#include "core/features/path_recorder.h"
#include "core/game_data_manager.h"
#include "core/settings_backup.h"
#include "core/hook/code_cave.h"
#include "utils/time.h"
#include "utils/memory.h"
#include "utils/types.h"
#include "utils/debug.h"

Freecam::Freecam(HMODULE hModule) : hModule(hModule) {
    instance = this;
}

bool Freecam::Initialize() {
    if (!config.Initialize(hModule, actionMgr)) return false;
    config.Reload();

    ModUtils::AttemptToGetWindowHandle();
    if (!ModUtils::muWindow) return false;

    if (!GameDataManager::Init()) return false;
    if (!input.HookWndProc(ModUtils::muWindow)) return false;

    if (!hookManager.Initialize()) return false;
    if (!hookManager.Hook(&GameDataManager::UpdateCameraMatrixFunc, &hkUpdateCameraMatrix, (void**)&origUpdateCameraMatrix))
        return false;
    if (!hookManager.Hook(&GetRawInputData, &Input::hkGetRawInputData, (void**)&Input::origGetRawInputData))
        return false;

    speedhack.Initialize(hookManager);

    if (!hookManager.EnableAll()) return false;

    if (!hookManager.GetDaytimeUpdateCave().Hook(GameDataManager::DaytimeUpdateFunc.address)) return false;

    SettingsBackup::SetFolderPath(config.GetConfigDirPath());

    freeCamera.Initialize();

    config.AddReloadCallback([this]() { freeCamera.OnConfigReload(); });

    return true;
}

void Freecam::Run() {
    if (!Initialize()) return;

    while (isRunning) {
        if (actionMgr.IsPressed(ActionType::ExitMod, input)) break;
        Sleep(10);
    }
}

void Freecam::ProcessInput(GameData::GameRend* gameRend, float deltaTime) {
    using enum ActionType;
    const float scrollDelta = input.GetScrollDelta();

    if (IsJustPressed(Toggle)) {
        config.Reload();
        freeCamera.Toggle(gameRend);
    }

    // TeleportToCamera
    if (IsJustPressed(TeleportToCamera)) {
        config.Reload();
        freeCamera.Toggle(gameRend);

        if (!freeCamera.IsEnabled()) {
            if (GameData::ChrIns* player = GameDataManager::GetPlayer()) {
                if (GameData::Camera* cam = gameRend->csDebugCam) {
                    player->chrModules->chrPhysics->localPos = cam->matrix.position();
                }
            }
        }
    }

    if (IsJustPressed(CycleWeatherTime)) {
        hookManager.GetDaytimeUpdateCave().ToggleCycleWeatherTime();
    }

    // FrameStepper
    if (IsPressed(StepFrames) || input.IsGamepadPressed(XINPUT_GAMEPAD_DPAD_DOWN)) {
        constexpr float holdWaitTime = 1.0f;
        if (frameStepperTimePressed <= 0 || frameStepperTimePressed >= holdWaitTime) {
            freeCamera.StepFrames();
        }
        frameStepperTimePressed += deltaTime;
    }
    else {
        frameStepperTimePressed = 0.0f;
    }

    // Speedhack
    if (IsJustPressed(ToggleSpeedhack)) speedhack.IsEnabled() ? speedhack.Disable() : speedhack.Enable();
    if (IsPressed(ScrollSpeedhackModifier) && speedhack.IsEnabled()) speedhack.AddTimeScale(scrollDelta * 0.05f);
    if (IsJustPressed(ResetSpeedhackSpeed)) speedhack.SetTimeScale(1.0);

    // Free camera only
    if (!freeCamera.IsEnabled()) return;

    freeCamera.SetMouseDelta(input.GetMouseDelta());
    freeCamera.SetGamepadDelta(input.GetThumbRight());

    freeCamera.SetIsSprinting(IsPressed(Sprint) || input.IsGamepadPressed(XINPUT_GAMEPAD_X));
    if (IsJustPressed(ToggleFreeze)) freeCamera.ToggleFreeze();
    if (IsJustPressed(ResetSettings) || input.IsGamepadJustPressed(XINPUT_GAMEPAD_Y)) freeCamera.ResetCameraState(gameRend);
    if (IsJustPressed(ReloadConfig)) config.Reload();

    if (input.IsGamepadJustPressed(XINPUT_GAMEPAD_DPAD_UP)) {
        hookManager.GetDaytimeUpdateCave().ToggleCycleWeatherTime();
    }

    if (IsJustPressed(StartEndRecording) || input.IsGamepadJustPressed(XINPUT_GAMEPAD_DPAD_LEFT)) freeCamera.GetPathRecorder().Record();
    if (IsJustPressed(StartEndPlayingRecording) || input.IsGamepadJustPressed(XINPUT_GAMEPAD_DPAD_RIGHT)) freeCamera.GetPathRecorder().PlayRecord();

    const float2 thumbLeft = input.GetThumbLeft();
    const float moveForward = IsPressed(MoveForward) - IsPressed(MoveBackward) + thumbLeft.y;
    const float moveRight   = IsPressed(MoveRight)   - IsPressed(MoveLeft)     + thumbLeft.x;
    const float moveUp      = IsPressed(MoveUp)      - IsPressed(MoveDown) 
                            + input.IsGamepadPressed(XINPUT_GAMEPAD_A)
                            - input.IsGamepadPressed(XINPUT_GAMEPAD_B);
	freeCamera.AddVelocity(float3(moveRight, moveUp, moveForward));

    freeCamera.AddRollVelocity(IsPressed(TiltLeft) - IsPressed(TiltRight) 
        + input.IsGamepadPressed(XINPUT_GAMEPAD_LEFT_SHOULDER) - input.IsGamepadPressed(XINPUT_GAMEPAD_RIGHT_SHOULDER));

    freeCamera.AddZoomVelocity(IsPressed(ZoomOut) - IsPressed(ZoomIn) 
        + input.GetLeftTrigger() - input.GetRightTrigger() - IsPressed(ScrollZoomModifier) * scrollDelta);

    if (IsPressed(ScrollCameraSpeedModifier)) freeCamera.AddSpeed(scrollDelta);

    // Number keys row
    if (input.IsPressed(VK_CONTROL)) {
        // Save states
        GameData::Camera* activeCamera = gameRend->GetActiveCamera();
        if (activeCamera) {
            for (int key = 0; key < 10; ++key) {
                int keyCode = key + (int)'0';

                if (input.IsJustPressed(keyCode)) {
                    freeCamera.GetCameraStateManager().SaveState(activeCamera, key, freeCamera.GetEuler());
                }
            }
        }
    }
    else {
        // Load states
        auto keysToProcess = input.GetReleasedNumkeys();
        if (!keysToProcess.empty()) {
            GameData::Camera* activeCamera = gameRend->GetActiveCamera();
            if (activeCamera) {
                freeCamera.GetCameraStateManager().StartLerpBetweenSlots(activeCamera, keysToProcess);
            }
        }
    }
}
    
void Freecam::Update(GameData::GameRend* gameRend) {
    if (input.IsWindowJustGetFocused()) {
        config.Reload();
    }

    float deltaTime = std::clamp(Time::DeltaTime(), 0.0f, 0.4f);
    input.UpdateGamepad();
    ProcessInput(gameRend, deltaTime);
    freeCamera.Update(gameRend, deltaTime);
    input.Reset();
}

void __fastcall Freecam::hkUpdateCameraMatrix(GameData::GameRend* gameRend, void* rdx, void* r8, void* r9) {
    origUpdateCameraMatrix(gameRend, rdx, r8, r9);
    
    if (instance && gameRend) instance->Update(gameRend);
}

void Freecam::Dispose() {
    LOG_INFO("Disposing Freecam...");
    isRunning = false;

    freeCamera.DisableCamera();
	speedhack.SetTimeScale(1.0f);
    hookManager.Shutdown();
    input.UnhookWndProc(ModUtils::muWindow);
    Logger::Shutdown();

    instance = nullptr;
}
