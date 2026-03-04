#include "freecam.h"
#include "mini/ini.h"
#include "core/game_data_manager.h"
#include "utils/time.h"
#include "utils/memory.h"
#include "utils/debug.h"
#include <iostream>

Freecam* Freecam::instance = nullptr;

Freecam::Freecam(HMODULE hModule, HWND hWnd) : hModule(hModule), hWnd(hWnd) {
    Logger::Info("Starting Freecam...");
    instance = this;
    last = std::chrono::high_resolution_clock::now();
}

void Freecam::ReloadConfig() {
	Logger::Info("Loading config...");
    mINI::INIFile file(config.configFilePath);
    mINI::INIStructure ini;

    bool fileExists = file.read(ini);

    freeCamera.SetDefaultSpeed(config.Read("settings", "default camera speed", 10.0f, ini));
    freeCamera.SetSpeedMult(config.Read("settings", "speed multiplier", 2.5f, ini));
    freeCamera.SetZoomSpeed(config.Read("settings", "zoom speed", 0.5f, ini));

	actionManager.BindAction(ActionType::Toggle, { config.Read("keybinds", "toggle", VK_F1, ini) });
	actionManager.BindAction(ActionType::ReloadConfig, { config.Read("keybinds", "reload config", VK_F5, ini) });
	actionManager.BindAction(ActionType::ExitMod, { config.Read("keybinds", "exit mod", VK_DELETE, ini) });
	actionManager.BindAction(ActionType::MoveForward, { config.Read("keybinds", "moveForward", 'W', ini)});
	actionManager.BindAction(ActionType::MoveBackward, { config.Read("keybinds", "moveBackward", 'S', ini)});
	actionManager.BindAction(ActionType::MoveLeft, { config.Read("keybinds", "moveLeft", 'A', ini) });
	actionManager.BindAction(ActionType::MoveRight, { config.Read("keybinds", "moveRight", 'D', ini) });
	actionManager.BindAction(ActionType::MoveUp, { config.Read("keybinds", "moveUp", VK_SPACE, ini) });
	actionManager.BindAction(ActionType::MoveDown, { config.Read("keybinds", "moveDown", VK_SHIFT, ini) });
	actionManager.BindAction(ActionType::ZoomIn, { config.Read("keybinds", "zoomIn", VK_OEM_PLUS, ini) });
	actionManager.BindAction(ActionType::ZoomOut, { config.Read("keybinds", "zoomOut", VK_OEM_MINUS, ini) });

    if (fileExists) {
        if (!file.write(ini)) Logger::Warn("Failed to write to config file");
    }
    else {
        if (!file.generate(ini, true)) Logger::Warn("Failed to generate config file");
    }
}

bool Freecam::Initialize() {
    if (!config.Initialize(hModule)) return false;
    ReloadConfig();

    if (!GameDataManager::Init()) return false;
    if (!input.HookWndProc(hWnd)) return false;
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
        if (actionManager.IsJustPressed(ActionType::ExitMod)) break;
		if (actionManager.IsJustPressed(ActionType::ReloadConfig)) ReloadConfig();
        Sleep(10);
    }

    Dispose();
}

void Freecam::Update(GameData::GameRend* gameRend) {
    float deltaTime = Time::DeltaTime();
    ProccesInput(gameRend);
    freeCamera.Update(gameRend, deltaTime);
    input.Reset();
}

void Freecam::ProccesInput(GameData::GameRend* gameRend) {
    if (actionManager.IsJustPressed(ActionType::Toggle)) {
        ReloadConfig();
        freeCamera.Toggle(gameRend);
    }

    if (input.IsMouseDown(Input::MouseButton::Left)) freeCamera.SetIsSprinting(true);

    if (actionManager.IsPressed(ActionType::MoveForward)) freeCamera.AddVelocity(float3::forward());
    if (actionManager.IsPressed(ActionType::MoveBackward)) freeCamera.AddVelocity(float3::back());
    if (actionManager.IsPressed(ActionType::MoveLeft)) freeCamera.AddVelocity(float3::left());
    if (actionManager.IsPressed(ActionType::MoveRight)) freeCamera.AddVelocity(float3::right());

    if (actionManager.IsPressed(ActionType::MoveUp)) freeCamera.AddVelocity(float3::up());
    if (actionManager.IsPressed(ActionType::MoveDown)) freeCamera.AddVelocity(float3::down());

    if (actionManager.IsPressed(ActionType::ZoomIn)) freeCamera.AddZoomVelocity(-1.0f);
    if (actionManager.IsPressed(ActionType::ZoomOut)) freeCamera.AddZoomVelocity(1.0f);
    if (input.IsPressed(VK_CONTROL)) freeCamera.AddZoomVelocity(-input.GetScrollDelta());
    else freeCamera.AddSpeed(input.GetScrollDelta());
}

void __fastcall Freecam::Hook_UpdateCameraMatrix(GameData::GameRend* gameRend,void* rdx, void* r8, void* r9) {
    originalUpdateCameraMatrix(gameRend, rdx, r8, r9);

    if (instance) {
        instance->Update(gameRend);
    }
    else {
		Logger::Warn("Freecam instance is null in Hook_UpdateCameraMatrix");
    }
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

    input.UnhookWndProc(hWnd);
}
