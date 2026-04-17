#pragma once
#include <windows.h>
#include <array>
#include <algorithm>

#include "MinSpeedhack.h"
#include "ModUtils.h"

#include "core/game_data_manager.h"

class Speedhack {
	double lastSpeed = 0.5;
	bool isEnabled = false;

	float* frametimeLimit = nullptr;
	float savedFrametimeLimit = 0;

public:
	bool Initialize() {
		uintptr_t framelimitAddress = GameDataManager::GetFrametimeLimitAddress();
		if (!framelimitAddress) return false;

		frametimeLimit = reinterpret_cast<float*>(framelimitAddress);
		if (!frametimeLimit) return false;

		uintptr_t fullscreenLimitAddress = GameDataManager::GetFullscreenLimit();
		if (!fullscreenLimitAddress) return false;

		// Removing 60 FPS fullscreen limit: https://github.com/techiew/EldenRingMods/blob/master/UnlockTheFps/DllMain.cpp
		return ModUtils::ReplaceExpectedBytesAtAddress(fullscreenLimitAddress, "c7 ? ef ? 00 00 00", "c7 45 ef 00 00 00 00");
	}

	float GetFrametimeLimit() const {
		if (frametimeLimit) return *frametimeLimit;
		return 0.0f;
	}

	void SetFrametimeLimit(float value) {
		if (frametimeLimit) *frametimeLimit = value;
	}

	float GetTimeScale() const { 
		return MS::GetSpeed(); 
	}

	void SetTimeScale(double scale) {
		scale = std::clamp(scale, 0.00005, 2.0);
		if (scale <= 1.0) {
			SetFrametimeLimit(savedFrametimeLimit * scale);
		}
		MS::SetSpeed(scale);
	}

	void AddTimeScale(double delta) {
		float decay = 1.0f;
		for (float i = 0.1f; i >= 0.0001f; i *= 0.1f) {
			if (MS::GetSpeed() > i) break;
			decay *= 0.1f;
		}

		SetTimeScale(MS::GetSpeed() + delta * decay);
	}

	bool IsEnabled() const { return isEnabled; }

	void Enable() {
		isEnabled = true;
		savedFrametimeLimit = GetFrametimeLimit();
		SetTimeScale(lastSpeed);
	}

	void Disable() {
		isEnabled = false;
		lastSpeed = MS::GetSpeed();
		SetTimeScale(1.0);
		SetFrametimeLimit(savedFrametimeLimit);
	}
};