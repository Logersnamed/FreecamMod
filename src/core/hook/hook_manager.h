#pragma once
#include <vector>
#include <functional>

#include "MinHook.h"
#include "ModUtils.h"

#include "core/hook/code_cave.h"
#include "utils/debug.h"

class HookManager {
    struct Hook {
        void* target;
        void* detour;
        void** original;
    };

    std::vector<Hook> hooks;
    bool isInitialized = false;

    DaytimeUpdateCave daytimeUpdateCave;

public:
    HookManager() {};
    ~HookManager() { Shutdown(); }

	DaytimeUpdateCave& GetDaytimeUpdateCave() { return daytimeUpdateCave; }

    bool Initialize();
    bool Hook(void* target, void* detour, void** original);
    bool EnableAll();
    void RemoveAll();
    void Shutdown();
};