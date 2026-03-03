#include "freecam.h"
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

bool Freecam::Initialize() {
    if (!GameDataManager::Init()) return false;
    if (!input.HookWndProc(hWnd)) return false;

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
        if (GetAsyncKeyState(VK_DELETE)) break;
        Sleep(10);
    }

    Dispose();
}

void Freecam::Update(GameData::GameRend* gameRend) {
    float deltaTime = Time::GetDeltaTime(std::chrono::high_resolution_clock::now(), &last);
    ProccesInput(gameRend);
    freeCamera.Update(gameRend, deltaTime);
    input.Reset();
}

void Freecam::ProccesInput(GameData::GameRend* gameRend) {
    if (input.IsJustPressed(VK_F1)) freeCamera.Toggle(gameRend);

    if (input.IsMouseDown(Input::MouseButton::Left)) freeCamera.SetIsSprinting(true);

    if (input.IsPressed('W')) freeCamera.AddVelocity(float3::forward());
    if (input.IsPressed('S')) freeCamera.AddVelocity(float3::back());
    if (input.IsPressed('A')) freeCamera.AddVelocity(float3::left());
    if (input.IsPressed('D')) freeCamera.AddVelocity(float3::right());

    if (input.IsPressed(VK_SPACE)) freeCamera.AddVelocity(float3::up());
    if (input.IsPressed(VK_SHIFT)) freeCamera.AddVelocity(float3::down());

    if (input.IsPressed(VK_OEM_PLUS)) freeCamera.AddZoomVelocity(-1.0f);
    if (input.IsPressed(VK_OEM_MINUS)) freeCamera.AddZoomVelocity(1.0f);
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
    isRunning = false;

	Logger::Info("Disabling free camera...");
    freeCamera.DisableCamera();

	Logger::Info("Unhooking UpdateCameraMatrixFunc...");
	Logger::Info("Uninitializing MinHook...");
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    input.UnhookWndProc(hWnd);
}
