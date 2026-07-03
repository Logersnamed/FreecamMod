#pragma once
#include "core/config/con_var.h"
#include "core/game_data/game_data.h"
#include "core/input/input.h"

class CameraStateManager {
	static constexpr int MAX_SLOTS = 10;

	struct State {
		float3 pos = 0.0f;
		Quaternion rotation{};
		float fov = 1.0f;
		EulerAngles yawPitchRoll{};
		bool isSaved = false;

		bool operator==(const State& other) const { return pos == other.pos && rotation == other.rotation && fov == other.fov; }
	} stateSlots[MAX_SLOTS]{};

	bool isInterpolating = false;
	Input::ReleasedNumkeys slotOrder;
	size_t interval = 0;
	float time = 0;

	ConVar<float> interpolationTime{ "camera_state_manager", "interpolation_time", 3.0f };

	void Lerp(GameData::Camera* camera, const State& startState, const State& endState, float t);

public:
	bool IsInterpolating() const { return isInterpolating; }
	Input::ReleasedNumkeys GetSlotOrder() const { return slotOrder; }
	size_t GetInterval() const { return interval; }
	float GetTime() const { return time; }
	float GetInterpolationTime() const { return interpolationTime; }
	void SetInterpolationTimeFromUI(float value) { interpolationTime.SetValueFromUI(value); }

	void SaveState(GameData::Camera* camera, int slot, const EulerAngles& yawPitchRoll);
	void StartLerpBetweenSlots(GameData::Camera* camera, Input::ReleasedNumkeys& positionSlots);
	void Update(GameData::Camera* camera, EulerAngles* yawPitchRoll, float dt);
};