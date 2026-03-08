#include "freecam.h"

#include "MinHook.h"
#include "mini/ini.h"
#include "ModUtils.h"

#include "core/game_data_manager.h"
#include "utils/time.h"
#include "utils/memory.h"
#include "utils/debug.h"

Freecam* Freecam::instance = nullptr;

Freecam::Freecam(HMODULE hModule) : hModule(hModule) {
    Logger::Info("Starting Freecam...");
    instance = this;
}

void Freecam::ReloadConfig() {
	Logger::Info("Loading config...");
    config.CreateModDirectory();

    mINI::INIFile file(config.GetConfigFilePath());
    mINI::INIStructure ini;

    bool fileExists = file.read(ini);

    Logger::Enable(config.ReadValue("mod", "debug", 0, ini));
    freeCamera.SetDefaultSpeed(config.ReadValue("settings", "default_camera_speed", 10.0f, ini));
    freeCamera.SetSpeedMult(config.ReadValue("settings", "speed_multiplier", 2.5f, ini));
    freeCamera.SetZoomSpeed(config.ReadValue("settings", "zoom_speed", 0.7f, ini));
	freeCamera.SetAutoDisableHud(config.ReadValue("settings", "hide_hud", 1, ini));
	freeCamera.SetDisableEnemiesMovement(config.ReadValue("settings", "disable_enemies_movement", 0, ini));
	freeCamera.SetEnableSmootherMovement(config.ReadValue("settings", "enable_smoother_camera_movement", 1, ini));

	const auto& keybinds = config.GetKeybinds();
    for (const auto& kb : keybinds) {
        actionManager.BindAction(config.ReadKeybind(
            kb.section, kb.name, kb.type, kb.defaultKeys, kb.defaultModifiers, ini
        ));
    }

    if (fileExists) {
        if (!file.write(ini, true)) Logger::Warn("Failed to write to config file");
    }
    else {
        if (!file.generate(ini, true)) Logger::Warn("Failed to generate config file");
    }
}

bool Freecam::Initialize() {
    if (!config.Initialize(hModule)) return false;
    ReloadConfig();

	Logger::Info("Attempting to get window handle...");
    ModUtils::AttemptToGetWindowHandle();
	if (!ModUtils::muWindow) {
        Logger::Error("Failed to obtain window handle");
		return false;
    }
    Logger::Info("Window handle obtained: %p", ModUtils::muWindow);

    if (!GameDataManager::Init()) return false;
    if (!input.HookWndProc(ModUtils::muWindow)) return false;
    if (!actionManager.Initialize(&input)) return false;

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

void Freecam::Run() {
    if (!Initialize()) {
        Dispose();
        return;
    }

    while (isRunning) {
        if (actionManager.IsJustPressed(Action::Type::ExitMod)) break;
        Sleep(10);
    }

    Dispose();
}

void Freecam::Update(GameData::GameRend* gameRend) {
    ProcessInput(gameRend);
    freeCamera.Update(gameRend, Time::DeltaTime());
    input.Reset();
}

void Freecam::ProcessInput(GameData::GameRend* gameRend) {
    if (actionManager.IsJustPressed(Action::Type::ReloadConfig)) ReloadConfig();

    if (actionManager.IsJustPressed(Action::Type::Toggle)) freeCamera.Toggle(gameRend);

    freeCamera.SetIsSprinting(actionManager.IsPressed(Action::Type::Sprint));
    
    if (actionManager.IsPressed(Action::Type::MoveForward)) freeCamera.AddVelocity(float3::forward());
    if (actionManager.IsPressed(Action::Type::MoveBackward)) freeCamera.AddVelocity(float3::back());
    if (actionManager.IsPressed(Action::Type::MoveLeft)) freeCamera.AddVelocity(float3::left());
    if (actionManager.IsPressed(Action::Type::MoveRight)) freeCamera.AddVelocity(float3::right());

    if (actionManager.IsPressed(Action::Type::MoveUp)) freeCamera.AddVelocity(float3::up());
    if (actionManager.IsPressed(Action::Type::MoveDown)) freeCamera.AddVelocity(float3::down());

    if (actionManager.IsPressed(Action::Type::ZoomIn)) freeCamera.AddZoomVelocity(-1.0f);
    if (actionManager.IsPressed(Action::Type::ZoomOut)) freeCamera.AddZoomVelocity(1.0f);

    if (actionManager.IsPressed(Action::Type::ScrollZoomModifier)) freeCamera.AddZoomVelocity(-input.GetScrollDelta());
    if (actionManager.IsPressed(Action::Type::ScroolCameraSpeedModifier)) freeCamera.AddSpeed(input.GetScrollDelta());
}

void __fastcall Freecam::Hook_UpdateCameraMatrix(GameData::GameRend* gameRend,void* rdx, void* r8, void* r9) {
    originalUpdateCameraMatrix(gameRend, rdx, r8, r9);

    instance->Update(gameRend);
}

void Freecam::Dispose() {
	Logger::Info("Disposing Freecam...");
    isRunning = false;

	Logger::Info("Disabling free camera...");
    freeCamera.DisableCamera();

	Logger::Info("Unhooking UpdateCameraMatrixFunc...");
	Logger::Info("Uninitializing MinHook...");
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    input.UnhookWndProc(ModUtils::muWindow);

    instance = nullptr;

    Logger::Info("Shutting down..");
    Logger::Shutdown();
}
