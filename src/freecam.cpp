#include "freecam.h"

#include <algorithm>
#include <optional>

#include "MinHook.h"
#include "ModUtils.h"

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
    if (!HookFunctions()) return false;

    SettingsBackup::SetFolderPath(config.GetConfigDirPath());
    if (auto restoredHudValue = SettingsBackup::RestoreHudValue()) {
        freeCamera.SetInitHudValue(restoredHudValue.value());
    }
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
    if (actionManager.IsPressed(Action::TiltLeft, input)) freeCamera.SetRollVeloctiy(1.0f);
    if (actionManager.IsPressed(Action::TiltRight, input)) freeCamera.SetRollVeloctiy(-1.0f);

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

    if (actionManager.IsPressed(Action::MoveUp, input)) freeCamera.AddVelocity(float3::up());
    if (actionManager.IsPressed(Action::MoveDown, input)) freeCamera.AddVelocity(float3::down());

    if (actionManager.IsPressed(Action::ZoomIn, input)) freeCamera.AddZoomVelocity(-1.0f);
    if (actionManager.IsPressed(Action::ZoomOut, input)) freeCamera.AddZoomVelocity(1.0f);

    if (actionManager.IsPressed(Action::ScrollZoomModifier, input)) freeCamera.AddZoomVelocity(-input.GetScrollDelta());
    if (actionManager.IsPressed(Action::ScrollCameraSpeedModifier, input)) freeCamera.AddSpeed(input.GetScrollDelta());
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

bool Freecam::HookFunctions() {
    Logger::Info("Initializing MinHook...");
    if (MH_Initialize() != MH_OK) {
        Logger::Error("Failed to initialize MinHook");
        return false;
    }

    Logger::Info("Scanning for UpdateCameraMatrixFunc...");
    UpdateCameraMatrix UpdateCameraMatrixFunc = (UpdateCameraMatrix)Signature("4C 8B 49 18 4C 8B D1 8B 42 50 41 89 41 50 8B 42").Scan().As<uint64_t>();
    if (!UpdateCameraMatrixFunc) {
        Logger::Error("Failed to find UpdateCameraMatrixFunc");
        return false;
    }
    Logger::Info("Hooking UpdateCameraMatrixFunc at %p...", UpdateCameraMatrixFunc);
    if (MH_CreateHook(UpdateCameraMatrixFunc, &Hook_UpdateCameraMatrix, (void**)&originalUpdateCameraMatrix) != MH_OK) {
        Logger::Error("Failed to create hook for UpdateCameraMatrixFunc");
        return false;
    }
    if (MH_EnableHook(UpdateCameraMatrixFunc) != MH_OK) {
        Logger::Error("Failed to enable hook for UpdateCameraMatrixFunc");
        return false;
    }

	return true;
}

void Freecam::Dispose() {
	Logger::Info("Disposing Freecam...");
    isRunning = false;

	Logger::Info("Disabling free camera...");
    freeCamera.DisableCamera();

	Logger::Info("Unhooking UpdateCameraMatrixFunc...");
    MH_RemoveHook(MH_ALL_HOOKS);
    Logger::Info("Uninitializing MinHook...");
    MH_Uninitialize();

    Logger::Info("Unhooking WndProc...");
    input.UnhookWndProc(ModUtils::muWindow);

    instance = nullptr;

    Logger::Info("Shutting down Logger..");
    Logger::Shutdown();
}
