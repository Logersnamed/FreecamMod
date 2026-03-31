#pragma once
#include <windows.h>

#include "core/input/action_system.h"
#include "core/input/input.h"
#include "core/config.h"
#include "core/free_camera.h"
#include "core/hook_manager.h"

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
	ActionManager actionManager{};
    HookManager hookManager{};

    bool isRunning = true;

    void Update(GameData::GameRend* gameRend);
	void ProcessInput(GameData::GameRend* gameRend, float deltaTime);
    void ProcessNumRowKeys(GameData::GameRend* gameRend);

    struct NumRowKey {
        bool shouldBeProcessed = false;
        int pressId = 0;
    } numRowKeys[10]{};

    int id = 0;
    bool isNumRowProcessed = true;
    bool isWaitingForOtherNumKeys = false;

    float frameStepperTimePressed = 0.0f;

    using updateCameraMatrix_t = void(__fastcall*)(void*, void*, void*, void*);
    static inline updateCameraMatrix_t origUpdateCameraMatrix{};
    static void __fastcall hkUpdateCameraMatrix(GameData::GameRend* gameRend, void* rdx, void* r8, void* r9);
};