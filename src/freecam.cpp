#include "freecam.h"

#include <algorithm>
#include <optional>

#include "ModUtils.h"

#include "core/features/path_recorder.h"
#include "core/game_data_manager.h"
#include "core/settings_backup.h"
#include "utils/time.h"
#include "utils/memory.h"
#include "utils/types.h"
#include "utils/debug.h"

Freecam::Freecam(HMODULE hModule) : hModule(hModule) {
    instance = this;
}

bool Freecam::Initialize() {
    if (!config.Initialize(hModule)) return false;
    config.Reload(actionManager, freeCamera);

    ModUtils::AttemptToGetWindowHandle();
	if (!ModUtils::muWindow) return false;

    if (!GameDataManager::Init()) return false;
    if (!input.HookWndProc(ModUtils::muWindow)) return false;

    if (!hookManager.Initialize()) return false;
    if (!hookManager.Create(GameDataManager::GetUpdateCameraMatrixFunc(), &Hook_UpdateCameraMatrix, (void**)&originalUpdateCameraMatrix)) 
        return false;
    if (!hookManager.EnableAll()) return false;

    SettingsBackup::SetFolderPath(config.GetConfigDirPath());
    freeCamera.SetHudValueToRestore(SettingsBackup::RestoreHudValue());
    SettingsBackup::SetEnabled(0);

    return true;
}

void Freecam::Run() {
    if (!Initialize()) return;

    while (isRunning) {
        if (actionManager.IsPressed(Action::ExitMod, input)) break;
        Sleep(10);
    }
}

void Freecam::ProcessInput(GameData::GameRend* gameRend) {
	freeCamera.SetMouseDelta(input.GetMouseDelta());

    if (actionManager.IsJustPressed(Action::ReloadConfig, input)) config.Reload(actionManager, freeCamera);
    if (actionManager.IsJustPressed(Action::ResetSettings, input)) freeCamera.ResetSettings(gameRend);
    if (actionManager.IsJustPressed(Action::Toggle, input)) {
        config.Reload(actionManager, freeCamera);
        freeCamera.Toggle(gameRend);
    }

    freeCamera.SetIsSprinting(actionManager.IsPressed(Action::Sprint, input));
    
    if (actionManager.IsPressed(Action::MoveForward, input)) freeCamera.AddVelocity(float3::forward());
    if (actionManager.IsPressed(Action::MoveBackward, input)) freeCamera.AddVelocity(float3::back());
    if (actionManager.IsPressed(Action::MoveLeft, input)) freeCamera.AddVelocity(float3::left());
    if (actionManager.IsPressed(Action::MoveRight, input)) freeCamera.AddVelocity(float3::right());

    if (actionManager.IsPressed(Action::TiltLeft, input)) freeCamera.AddRollVelocity(1.0f);
    if (actionManager.IsPressed(Action::TiltRight, input)) freeCamera.AddRollVelocity(-1.0f);

    if (actionManager.IsPressed(Action::MoveUp, input)) freeCamera.AddVelocity(float3::up());
    if (actionManager.IsPressed(Action::MoveDown, input)) freeCamera.AddVelocity(float3::down());

    if (actionManager.IsPressed(Action::ZoomIn, input)) freeCamera.AddZoomVelocity(-1.0f);
    if (actionManager.IsPressed(Action::ZoomOut, input)) freeCamera.AddZoomVelocity(1.0f);

    if (actionManager.IsPressed(Action::ScrollZoomModifier, input)) freeCamera.AddZoomVelocity(-input.GetScrollDelta());
    if (actionManager.IsPressed(Action::ScrollCameraSpeedModifier, input)) freeCamera.AddSpeed(input.GetScrollDelta());

    if (actionManager.IsJustPressed(Action::StartEndRecording, input)) freeCamera.GetPathRecorder().Record();
    if (actionManager.IsJustPressed(Action::StartEndPlayingRecording, input)) freeCamera.GetPathRecorder().PlayRecord();

    if (actionManager.IsJustPressed(Action::StepFrames, input)) freeCamera.StepFrames();

    ProcessNumRowKeys(gameRend);
}

void Freecam::ProcessNumRowKeys(GameData::GameRend* gameRend) {
    if (input.IsPressed(VK_CONTROL)) {
        GameData::Camera* activeCamera = gameRend->IsFreecamEnabled() ? gameRend->csDebugCam : gameRend->csPersCam1;
        if (!activeCamera) return;

        for (int key = 0; key < 10; ++key) {
            int keyCode = key + (int)'0';

            if (input.IsJustPressed(keyCode)) {
                freeCamera.GetCameraStateManager().SaveState(activeCamera, key, freeCamera.GetYawPitchRoll());
            }
        }

        return;
    }

    bool isAtLeastOnePressed = false;
    for (int key = 0; key < 10; ++key) {
        int keyCode = key + (int)'0';

        if (input.IsPressed(keyCode)) {
            isAtLeastOnePressed = true;
            isWaitingForOtherNumKeys = true;

            numRowKeys[key].shouldBeProcessed = false;
            if (input.IsJustPressed(keyCode)) {
                numRowKeys[key].pressId = id++;
            }
        }

        if (input.IsReleased(keyCode)) {
            if (isWaitingForOtherNumKeys) isNumRowProcessed = false;

            numRowKeys[key].shouldBeProcessed = true;
        }
    }

    std::vector<uint8_t> keysToProcess;
    if (!isNumRowProcessed && !isAtLeastOnePressed) {
        for (int key = 0; key < 10; ++key) {
            if (numRowKeys[key].shouldBeProcessed) {
                numRowKeys[key].shouldBeProcessed = false;
                keysToProcess.push_back(key);
            }
        }

        isNumRowProcessed = true;
        isWaitingForOtherNumKeys = false;
        id = 0;
    }

    if (keysToProcess.size()) {
        std::sort(keysToProcess.begin(), keysToProcess.end(), [&](const int& a, const int& b) {
            return numRowKeys[a].pressId < numRowKeys[b].pressId;
            });

        GameData::Camera* activeCamera = gameRend->IsFreecamEnabled() ? gameRend->csDebugCam : gameRend->csPersCam1;
        freeCamera.GetCameraStateManager().StartLerpBetweenSlots(activeCamera, keysToProcess);
    }
}

void Freecam::Update(GameData::GameRend* gameRend) {
    if (input.IsWindowJustGetFocused()) {
        config.Reload(actionManager, freeCamera);
    }

    ProcessInput(gameRend);
    freeCamera.Update(gameRend, std::clamp(Time::DeltaTime(), 0.0f, 0.4f));
    input.Reset();
}

void __fastcall Freecam::Hook_UpdateCameraMatrix(GameData::GameRend* gameRend,void* rdx, void* r8, void* r9) {
    originalUpdateCameraMatrix(gameRend, rdx, r8, r9);

    if (instance && gameRend) instance->Update(gameRend);
}

void Freecam::Dispose() {
    LOG_INFO("Disposing Freecam...");
    isRunning = false;

    freeCamera.DisableCamera();
    hookManager.Shutdown();
    input.UnhookWndProc(ModUtils::muWindow);
    Logger::Shutdown();

    instance = nullptr;
}
