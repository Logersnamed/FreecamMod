#include "freecam.h"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "ModUtils.h"
#include "MinSpeedhack.h"

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
    if (!config.Initialize(hModule)) return false;  // 3
    config.Reload(actionMgr, freeCamera);           // 11

    ModUtils::AttemptToGetWindowHandle();
    if (!ModUtils::muWindow) return false;

    if (!GameDataManager::Init()) return false;
    if (!input.HookWndProc(ModUtils::muWindow)) return false;

    if (!hookManager.Initialize()) return false;
    if (!hookManager.Hook(&GameDataManager::UpdateCameraMatrixFunc, &hkUpdateCameraMatrix, (void**)&origUpdateCameraMatrix))
        return false;
    if (!hookManager.Hook(&GetRawInputData, &Input::hkGetRawInputData, (void**)&Input::origGetRawInputData))
        return false;

    size_t hookCount = 0;
    const auto* hooks = MS::GetHooks(hookCount);
    for (size_t i = 0; i < hookCount; ++i) {
        if (!hookManager.Hook(hooks[i].target, hooks[i].detour, hooks[i].original))
            return false;
    }

    if (!hookManager.EnableAll()) return false;

    if (!hookManager.GetDaytimeUpdateCave().Hook(GameDataManager::DaytimeUpdateFunc.address)) return false;

    speedhack.Initialize();

    SettingsBackup::SetFolderPath(config.GetConfigDirPath());

    freeCamera.Initialize();

    return true;
}

void Freecam::Run() {
    if (!Initialize()) return;

    while (isRunning) {
        if (actionMgr.IsPressed(Action::ExitMod, input)) break;
        Sleep(10);
    }
}

void Freecam::ProcessInput(GameData::GameRend* gameRend, float deltaTime) {
    bool isFreecamEnabled = gameRend->IsFreecamEnabled();
	float scrollDelta = input.GetScrollDelta();

    if (actionMgr.IsJustPressed(Action::Toggle, input)) {
        config.Reload(actionMgr, freeCamera);
        freeCamera.Toggle(gameRend);
    }

    // TeleportToCamera
    if (actionMgr.IsJustPressed(Action::TeleportToCamera, input)) {
        config.Reload(actionMgr, freeCamera);
        freeCamera.Toggle(gameRend);

        if (!freeCamera.IsEnabled()) {
            if (GameData::ChrIns* player = GameDataManager::GetPlayer()) {
                if (GameData::Camera* cam = gameRend->csDebugCam) {
                    player->chrModules->chrPhysics->localPos = cam->matrix.position();
                }
            }
        }
    }

    if (actionMgr.IsJustPressed(Action::CycleWeatherTime, input)) {
        hookManager.GetDaytimeUpdateCave().ToggleCycleWeatherTime();
    }

    // FrameStepper
    if (actionMgr.IsPressed(Action::StepFrames, input)) {
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
    if (actionMgr.IsJustPressed(Action::ToggleSpeedhack, input)) 
        speedhack.IsEnabled() ? speedhack.Disable() : speedhack.Enable();
    if (actionMgr.IsPressed(Action::ScrollSpeedhackModifier, input) && speedhack.IsEnabled()) 
        speedhack.AddTimeScale(scrollDelta * 0.05f);
    if (actionMgr.IsJustPressed(Action::ResetSpeedhackSpeed, input)) speedhack.SetTimeScale(1.0);

    // Free camera only
    if (!isFreecamEnabled) return;

    freeCamera.SetMouseDelta(input.GetMouseDelta());
    freeCamera.SetIsSprinting(actionMgr.IsPressed(Action::Sprint, input));

    if (actionMgr.IsJustPressed(Action::ToggleFreeze, input)) freeCamera.ToggleFreeze();
    if (actionMgr.IsJustPressed(Action::ResetSettings, input)) freeCamera.ResetSettings(gameRend);
    if (actionMgr.IsJustPressed(Action::ReloadConfig, input)) config.Reload(actionMgr, freeCamera);

    if (actionMgr.IsPressed(Action::MoveForward, input)) freeCamera.AddVelocity(float3::forward());
    if (actionMgr.IsPressed(Action::MoveBackward, input)) freeCamera.AddVelocity(float3::back());
    if (actionMgr.IsPressed(Action::MoveLeft, input)) freeCamera.AddVelocity(float3::left());
    if (actionMgr.IsPressed(Action::MoveRight, input)) freeCamera.AddVelocity(float3::right());
    if (actionMgr.IsPressed(Action::MoveUp, input)) freeCamera.AddVelocity(float3::up());
    if (actionMgr.IsPressed(Action::MoveDown, input)) freeCamera.AddVelocity(float3::down());
    if (actionMgr.IsPressed(Action::TiltLeft, input)) freeCamera.AddRollVelocity(1.0f);
    if (actionMgr.IsPressed(Action::TiltRight, input)) freeCamera.AddRollVelocity(-1.0f);

    if (actionMgr.IsPressed(Action::ZoomIn, input)) freeCamera.AddZoomVelocity(-1.0f);
    if (actionMgr.IsPressed(Action::ZoomOut, input)) freeCamera.AddZoomVelocity(1.0f);

    if (actionMgr.IsPressed(Action::ScrollZoomModifier, input)) freeCamera.AddZoomVelocity(-scrollDelta);
    if (actionMgr.IsPressed(Action::ScrollCameraSpeedModifier, input)) freeCamera.AddSpeed(scrollDelta);

    if (actionMgr.IsJustPressed(Action::StartEndRecording, input)) freeCamera.GetPathRecorder().Record();
    if (actionMgr.IsJustPressed(Action::StartEndPlayingRecording, input)) freeCamera.GetPathRecorder().PlayRecord();

    // Number keys row
    if (input.IsPressed(VK_CONTROL)) {
        // Save states
        GameData::Camera* activeCamera = gameRend->GetActiveCamera();
        if (activeCamera) {
            for (int key = 0; key < 10; ++key) {
                int keyCode = key + (int)'0';

                if (input.IsJustPressed(keyCode)) {
                    freeCamera.GetCameraStateManager().SaveState(activeCamera, key, freeCamera.GetYawPitchRoll());
                }
            }
        }
    }
    else {
        // Load states
        std::vector<uint8_t> keysToProcess = input.GetReleasedNumkeysInOrder();
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
        config.Reload(actionMgr, freeCamera);
    }

    float deltaTime = std::clamp(Time::DeltaTime() / speedhack.GetTimeScale(), 0.0f, 0.4f);
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
