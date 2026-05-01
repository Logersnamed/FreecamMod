#pragma once
#include <windows.h>

#include "core/features/speedhack.h"
#include "core/game_data/game_data.h"
#include "core/input/action_system.h"
#include "core/input/input.h"
#include "core/config.h"
#include "core/free_camera.h"
#include "core/hook/hook_manager.h"

class Freecam {
public:
    static inline Freecam* instance = nullptr;

    Freecam(HMODULE hModule);

    bool Initialize();
    void Run();
    void Dispose();

private:
    HMODULE hModule{};
    FreeCamera freeCamera{};
    Config config{};
    Input input{};
    ActionManager actionMgr{};
    HookManager hookManager{};
    Speedhack speedhack{};

    bool isRunning = true;

    void Update(GameData::GameRend* gameRend);
    void ProcessInput(GameData::GameRend* gameRend, float deltaTime);

    bool IsPressed(ActionType actionType) const { return actionMgr.IsPressed(actionType, input); }
    bool IsJustPressed(ActionType actionType) const { return actionMgr.IsJustPressed(actionType, input); }

    float frameStepperTimePressed = 0.0f;

    using updateCameraMatrix_t = void(__fastcall*)(void*, void*, void*, void*);
    static inline updateCameraMatrix_t origUpdateCameraMatrix{};
    static void __fastcall hkUpdateCameraMatrix(GameData::GameRend* gameRend, void* rdx, void* r8, void* r9);
};