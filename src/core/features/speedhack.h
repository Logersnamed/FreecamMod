#pragma once
#include "MinSpeedhack.h"

#include "core/game_data_manager.h"
#include "core/config/con_var.h"

class HookManager;

class Speedhack {
	double targetSpeed = 0.5;
	bool isEnabled = false;

	float* frametimeLimit = nullptr;
	float savedFrametimeLimit = 0;

	bool isInitialized = false;

	ConVar<bool> isFreecamOnly{ "features_work_only_in_freecam", "speedhack", true };

	void ApplySpeed(double scale);

	HookManager* hookManager{};

	bool OnFirstEnable();

public:
	bool Initialize(HookManager& hookManager);

	void Enable();
	void Disable();

	bool IsFreecamOnly() const { return isFreecamOnly; }
	bool IsEnabled() const { return isEnabled; }

	void SetSpeed(double scale);
	void AddSpeed(double delta);

	void SetFrametimeLimit(float value) { if (frametimeLimit) *frametimeLimit = value; }
	float GetSpeedhackSpeed() const { return targetSpeed; }
	float GetGameSpeed() const { return MS::GetSpeed(); }
	float GetFrametimeLimit() const;
};