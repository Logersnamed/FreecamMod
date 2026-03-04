#pragma once
#include <windows.h>
#include <chrono>
#include <MinHook.h>

#include "core/free_camera.h"
#include "core/input.h"
#include "core/action.h"

class Freecam {
public:
    static Freecam* instance;
    bool isRunning = true;

    Freecam(HMODULE hModule, HWND hWnd);
    bool Initialize();
    void Run();
    void Dispose();
    void Update(GameData::GameRend* gameRend);

    HMODULE GetModule() const { return hModule; };
private:
    HMODULE hModule{};
    HWND hWnd{};
    FreeCamera freeCamera{};
    Input input{};
	ActionManager actionManager{};

	void ProccesInput(GameData::GameRend* gameRend);

    std::chrono::time_point<std::chrono::high_resolution_clock> last;

    using UpdateCameraMatrix = void(__fastcall*)(void*, void*, void*, void*);
    static inline UpdateCameraMatrix originalUpdateCameraMatrix{};
    static void __fastcall Hook_UpdateCameraMatrix(GameData::GameRend* gameRend, void* rdx, void* r8, void* r9);
};