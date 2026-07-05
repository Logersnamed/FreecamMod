#pragma once
#include "hook/hook_manager.h"
#include "core/mod_context.h"
#include "core/config/config.h"
#include "core/features/speedhack.h"
#include "core/game_data/game_data.h"
#include "core/input/action_system.h"
#include "core/input/input.h"
#include "core/free_camera.h"
#include "gui/gui.h"

class Freecam {
public:
    static inline Freecam* instance = nullptr;

    Freecam(HMODULE hModule);

    bool Initialize();
    void Run();
    void Dispose();

    void OnConfigReload();

private:
    HMODULE hModule{};

    FreeCamera    freeCamera{};
    Config        cfg{};
    Input         input{};
    ActionManager actionMgr{ input };
    HookManager   hookManager{};
    Speedhack     speedhack{};

    ModContext context{ cfg, input, actionMgr, hookManager, speedhack, freeCamera };
    GUI gui{ context };

    bool isRunning = true;

    void Update(GameData::GameRend* gameRend);
    void ProcessInput(GameData::GameRend* gameRend, float deltaTime);

    bool IsPressed(ActionType actionType) const { return actionMgr.IsPressed(actionType); }
    bool IsJustPressed(ActionType actionType) const { return actionMgr.IsJustPressed(actionType); }

    void ToggleFreecam(GameData::GameRend* gameRend);

    float frameStepperTimePressed = 0.0f;

    using updateCameraMatrix_t = void(__fastcall*)(void*, void*, void*, void*);
    static inline updateCameraMatrix_t origUpdateCameraMatrix{};
    static void __fastcall hkUpdateCameraMatrix(GameData::GameRend* gameRend, void* rdx, void* r8, void* r9);
};