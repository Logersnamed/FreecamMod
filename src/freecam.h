#pragma once
#include <windows.h>

#include "core/input/action.h"
#include "core/input/input.h"
#include "core/config.h"
#include "core/free_camera.h"

class Freecam {
public:
    static Freecam* instance;

    Freecam(HMODULE hModule);
    bool Initialize();
    void Run();
    void Dispose();

private:
    HMODULE hModule{};
    FreeCamera freeCamera{};
    Config config{};
    Input input{};
	ActionManager actionManager{};

    bool isRunning = true;

    void Update(GameData::GameRend* gameRend);
	void ProcessInput(GameData::GameRend* gameRend);

    bool HookFunctions();

    using UpdateCameraMatrix = void(__fastcall*)(void*, void*, void*, void*);
    static inline UpdateCameraMatrix originalUpdateCameraMatrix{};
    static void __fastcall Hook_UpdateCameraMatrix(GameData::GameRend* gameRend, void* rdx, void* r8, void* r9);
};