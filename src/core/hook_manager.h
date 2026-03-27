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
        LOG_INFO("Initializing MinHook...");
        if (MH_Initialize() != MH_OK) {
            LOG_ERROR("MH_Initialize failed");
            return false;
        }
        return true;
    }

    bool Create(void* target, void* detour, void** original) {
        if (MH_CreateHook(target, detour, original) != MH_OK) {
            LOG_ERROR("CreateHook failed (target = %p)", target);
            return false;
        }
        LOG_INFO("Hook created (target = %p)", target);
        hooks.push_back({ target, detour, original });
        return true;
    }

    bool EnableAll() {
        LOG_INFO("Enabling hooks...");
        for (auto& h : hooks) {
            if (MH_EnableHook(h.target) != MH_OK) {
                LOG_ERROR("EnableHook failed (target = %p)", h.target);
                return false;
            }
        }
        return true;
    }

    void RemoveAll() {
        for (auto& h : hooks) {
            MH_RemoveHook(h.target);
            LOG_INFO("Hook removed (target = %p)", h.target);
        }
        hooks.clear();
    }

    void Shutdown() {
        LOG_INFO("Shutting down hookManager...");
        RemoveAll();
        MH_Uninitialize();
    }
};