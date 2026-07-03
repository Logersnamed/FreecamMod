#include "hook/hook_manager.h"

bool HookManager::Initialize() {
    if (isInitialized) {
        LOG_WARN("HookManager already initialized");
        return true;
    }

    LOG_INFO("Initializing MinHook...");

    MH_STATUS status = MH_Initialize();

    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
        LOG_ERROR("MH_Initialize failed: %s", MH_StatusToString(status));
        return false;
    }

    isInitialized = true;
    return true;
}

bool HookManager::Enable(void* target) {
    if (!isInitialized) return false;

    MH_STATUS status = MH_EnableHook(target);
    if (status == MH_ERROR_ENABLED) {
        LOG_WARN("MH_EnableHook: Hook for target %p is already enabled", target);
        return true;
    }

    if (status != MH_OK) {
        LOG_ERROR("MH_EnableHook failed (target=%p): %s", target, MH_StatusToString(status));
        return false;
    }

    return true;
}

bool HookManager::Hook(void* target, void* detour, void** original) {
    if (!isInitialized) return false;

    MH_STATUS status = MH_CreateHook(target, detour, original);
    if (status == MH_ERROR_ALREADY_CREATED) {
        LOG_WARN("Hook already created (target=%p)", target);
        return true;
    }

    if (status != MH_OK) {
        LOG_ERROR("MH_CreateHook failed (target=%p detour=%p): %s", target, detour, MH_StatusToString(status));
        return false;
    }

    LOG_INFO("Hook created (target = %p)", target);
    hooks.push_back({ target, detour, original });
    return true;
}

bool HookManager::HookAndEnable(void* target, void* detour, void** original) {
    if (!Hook(target, detour, original)) return false;
	if (!Enable(target)) return false;
    return true;
}

bool HookManager::EnableAll() {
    LOG_INFO("Enabling hooks...");
    for (auto& hook : hooks) {
		if (!Enable(hook.target))
			return false;
    }
    return true;
}

void HookManager::RemoveAll() {
    for (auto& h : hooks) {
        MH_STATUS status = MH_RemoveHook(h.target);
        if (status != MH_OK) {
            LOG_WARN("MH_RemoveHook failed (target=%p): %s", h.target, MH_StatusToString(status));
        }
    }
    hooks.clear();
}

void HookManager::Shutdown() {
    if (!isInitialized) return;
    LOG_INFO("Shutting down hookManager...");
    RemoveAll();
    MH_Uninitialize();

    daytimeUpdateCave.Unhook();

    isInitialized = false;
}