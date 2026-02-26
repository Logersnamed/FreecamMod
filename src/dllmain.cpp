#include <windows.h>
#include <iostream>
#include <chrono>

#include <MinHook.h>

#include "core/game_data_manager.h"
#include "free_camera.h"

float GetDeltaTime(std::chrono::steady_clock::time_point now, std::chrono::steady_clock::time_point* last) {
    std::chrono::duration<float> elapsed = now - *last;
    *last = now;

    float deltaTime = elapsed.count();
    if (deltaTime > 0.1f)
        deltaTime = 1.0f / 60.0f;

    return deltaTime;
}

inline HMODULE g_Module{};
bool isRunning = true;
DWORD WINAPI MainThread(LPVOID lpParam) {
    using clock = std::chrono::high_resolution_clock;
	auto last = clock::now();

    if (!GameDataManager::Init()) return 0;

    FreeCamera freeCamera;
	while (isRunning) {
        if (GetAsyncKeyState(VK_DELETE) & 0x8000) break;

		float deltaTime = GetDeltaTime(clock::now(), &last);

		freeCamera.Update(deltaTime);

        Sleep(1);
	}

	Sleep(500);
    FreeLibraryAndExitThread(g_Module, 0);
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
            isRunning = FALSE;
            break;
    }

	return TRUE;
}