#pragma once
#include <vector>
#include <functional>

#include "MinHook.h"
#include "utils/debug.h"

class HookManager {
    struct Hook {
        void* target;
        void* detour;
        void** original;
    };

    std::vector<Hook> hooks;

public:
    bool Initialize() {
        Logger::Info("Initializing MinHook...");
        if (MH_Initialize() != MH_OK) {
            Logger::Error("MH_Initialize failed");
            return false;
        }
        return true;
    }

    bool Create(void* target, void* detour, void** original) {
        if (MH_CreateHook(target, detour, original) != MH_OK) {
            Logger::Error("CreateHook failed (target = %p)", target);
            return false;
        }
        Logger::Info("Hook created (target = %p)", target);
        hooks.push_back({ target, detour, original });
        return true;
    }

    bool EnableAll() {
        Logger::Info("Enabling hooks...");
        for (auto& h : hooks) {
            if (MH_EnableHook(h.target) != MH_OK) {
                Logger::Error("EnableHook failed (target = %p)", h.target);
                return false;
            }
        }
        return true;
    }

    void RemoveAll() {
        for (auto& h : hooks) {
            MH_RemoveHook(h.target);
            Logger::Info("Hook removed (target = %p)", h.target);
        }
        hooks.clear();
    }

    void Shutdown() {
        Logger::Info("Shutting down hookManager...");
        RemoveAll();
        MH_Uninitialize();
    }
};