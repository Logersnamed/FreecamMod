#include "freecam.h"
#include "freecam.h"
#include "universal_wndproc_hook.h"
#include "core/game_data_manager.h"
#include "utils/time.h"
#include "utils/memory.h"

Freecam* Freecam::instance = nullptr;

Freecam::Freecam(HMODULE hModule, HWND hWnd) : hModule(hModule), hWnd(hWnd) {
    instance = this;
    last = std::chrono::high_resolution_clock::now();
}

bool Freecam::Initialize() {
    if (!GameDataManager::Init()) return false;

    UWPH::HookWndProc(hWnd);

    MH_Initialize();
     
    UpdateCameraMatrix UpdateCameraMatrixFunc = (UpdateCameraMatrix)Signature("4C 8B 49 18 4C 8B D1 8B 42 50 41 89 41 50 8B 42").Scan().As<uint64_t>();
    if (!UpdateCameraMatrixFunc) return false;

    if (MH_CreateHook(UpdateCameraMatrixFunc, &Hook_UpdateCameraMatrix, (void**)&originalUpdateCameraMatrix) != MH_OK) return false;
    if (MH_EnableHook(UpdateCameraMatrixFunc) != MH_OK) return false;

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
    Sleep(500);
    FreeLibraryAndExitThread(hModule, 0);
}

void Freecam::Dispose() {
	isRunning = false;

    freeCamera.DisableCamera(
        GameDataManager::GetFieldArea()->gameRend,
        GameDataManager::GetPlayer()
    );

    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    UWPH::UnhookWndProc(hWnd);
}

void Freecam::Update(GameData::GameRend* gameRend) {
    float deltaTime = Time::GetDeltaTime(std::chrono::high_resolution_clock::now(), &last);

    freeCamera.Update(gameRend, deltaTime);
}

void __fastcall Freecam::Hook_UpdateCameraMatrix(GameData::GameRend* gameRend,void* rdx, void* r8, void* r9) {
    originalUpdateCameraMatrix(gameRend, rdx, r8, r9);

    if (instance) instance->Update(gameRend);
}