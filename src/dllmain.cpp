#include <windows.h>
#include <iostream>
#include <chrono>

#include <MinHook.h>
#include "utils/memory.h"
#include "utils/debug.h"
#include "utils/time.h"

#include "core/game_data_manager.h"
#include "core/free_camera.h"

#define HOT_UNLOAD_ENABLED

#ifdef HOT_UNLOAD_ENABLED
bool isRunning = true;
#endif

inline HMODULE g_Module{};

FreeCamera freeCamera;
std::chrono::time_point last = std::chrono::high_resolution_clock::now();

using UpdateCameraMatrix = void(__fastcall*)(void*, void*, void*, void*);
UpdateCameraMatrix originalUpdateCameraMatrix = nullptr;

void __fastcall Hook_UpdateCameraMatrix(GameData::GameRend* gameRend, void* rdx, void* r8, void* r9) {
    originalUpdateCameraMatrix(gameRend, rdx, r8, r9);

    float deltaTime = Time::GetDeltaTime(std::chrono::high_resolution_clock::now(), &last);
    freeCamera.Update(gameRend, deltaTime);
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    if (!GameDataManager::Init()) return 0;

    MH_Initialize();

    UpdateCameraMatrix UpdateCameraMatrixFunc = (UpdateCameraMatrix)Signature("4C 8B 49 18 4C 8B D1 8B 42 50 41 89 41 50 8B 42").Scan().As<uint64_t>();
    if (!UpdateCameraMatrixFunc) return 0;

    if (MH_CreateHook(UpdateCameraMatrixFunc, &Hook_UpdateCameraMatrix, (void**)&originalUpdateCameraMatrix) != MH_STATUS::MH_OK) return 0;
    if (MH_EnableHook(UpdateCameraMatrixFunc) != MH_OK) return 0;

#ifdef HOT_UNLOAD_ENABLED
	while (isRunning) {
         if (GetAsyncKeyState(VK_DELETE)) break;
         Sleep(1000);
	}

    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

	Sleep(500);
    FreeLibraryAndExitThread(g_Module, 0);
#endif // HOT_UNLOAD_ENABLED

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    if (!g_Module) g_Module = module;

    switch (reason) {
        case (DLL_PROCESS_ATTACH):  
            DisableThreadLibraryCalls(module);
            CreateThread(0, 0, &MainThread, 0, 0, NULL);
            break;
        case (DLL_PROCESS_DETACH):  
#ifdef HOT_UNLOAD_ENABLED
            isRunning = FALSE;
#else
            MH_RemoveHook(MH_ALL_HOOKS);
            MH_Uninitialize();
#endif // HOT_UNLOAD_ENABLED
            break;
    }

	return TRUE;
}