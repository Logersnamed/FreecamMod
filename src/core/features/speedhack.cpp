#include "core/features/speedhack.h"
#include "core/events.h"
#include "hook/hook_manager.h"
#include "ModUtils.h"

void Speedhack::ApplySpeed(double scale) {
	if (scale <= 1.0) {
		SetFrametimeLimit(savedFrametimeLimit * scale);
	}
	MS::SetSpeed(scale);
}

void Speedhack::SetSpeed(double scale) {
	scale = std::clamp(scale, 0.00005, 2.0);
	targetSpeed = scale;

	if (isEnabled) {
		ApplySpeed(targetSpeed);
	}
}

bool Speedhack::Initialize(HookManager& hookManager) {
	this->hookManager = &hookManager;

	uintptr_t framelimitAddress = GameDataManager::FrametimeLimit.Get();
	if (!framelimitAddress) return false;

	frametimeLimit = reinterpret_cast<float*>(framelimitAddress);
	if (!frametimeLimit) return false;

	uintptr_t fullscreenLimitAddress = GameDataManager::FullscreenLimit.Get();
	if (fullscreenLimitAddress) {
		// Removing 60 FPS fullscreen limit: https://github.com/techiew/EldenRingMods/blob/master/UnlockTheFps/DllMain.cpp
		ModUtils::ReplaceExpectedBytesAtAddress(fullscreenLimitAddress, "c7 ? ef ? 00 00 00", "c7 45 ef 00 00 00 00");
	}

	isInitialized = true;
	return true;
}

float Speedhack::GetFrametimeLimit() const {
	if (frametimeLimit) return *frametimeLimit;
	return 0.0f;
}

void Speedhack::AddSpeed(double delta) {
	float decay = 1.0f;
	for (float i = 0.1f; i >= 0.0001f; i *= 0.1f) {
		if (MS::GetSpeed() > i) break;
		decay *= 0.1f;
	}

	SetSpeed(MS::GetSpeed() + delta * decay);
}

bool Speedhack::OnFirstEnable() {
	size_t hookCount = 0;
	const auto* hooks = MS::GetHooks(hookCount);
	for (size_t i = 0; i < hookCount; ++i) {
		if (!hookManager->HookAndEnable(hooks[i].target, hooks[i].detour, hooks[i].original)) {
			return false;
		}
	}

	// Speedhack only "eldenring.exe" module
	MS::IncludeModule(GetModuleHandleW(NULL));

	return true;
}

void Speedhack::Enable() {
	if (!isInitialized || isEnabled) return;

	static bool isFirstEnable = true;
	if (isFirstEnable) {
		if (!OnFirstEnable()) {
			isInitialized = false;
			return;
		}
		isFirstEnable = false;
	}

	isEnabled = true;
	savedFrametimeLimit = GetFrametimeLimit();
	ApplySpeed(targetSpeed);

	EventBus::Emit(Event::ToggleSpeedhack{ .isEnabled = true });
}

void Speedhack::Disable() {
	if (!isEnabled) return;

	isEnabled = false;
	ApplySpeed(1.0);
	SetFrametimeLimit(savedFrametimeLimit);

	EventBus::Emit(Event::ToggleSpeedhack{ .isEnabled = false });
}