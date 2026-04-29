#pragma once
#include <iostream>
#include <vector>
#include <cstdint>

#include "core/game_data/game_data.h"
#include "utils/types.h"
#include "utils/math.h"
#include "utils/debug.h"

class CameraStateManager {
	static constexpr int MAX_SLOTS = 10;

	struct State {
		float3 pos = 0.0f;
		Quaternion rotation{};
		float fov = 1.0f;
		Rotation yawPitchRoll{};
		bool isSaved = false;

		bool operator==(const State& other) const { return pos == other.pos && rotation == other.rotation && fov == other.fov; }
	} stateSlots[MAX_SLOTS]{};

	bool isInterpolating = false;
	std::vector<uint8_t> slotOrder;
	uint8_t interval = 0;
	float time = 0;
	float iTime = 3.0f;

public:
	void SetInterpolationTime(float newTime) { iTime = newTime; }

	void SaveState(GameData::Camera* camera, int slot, const Rotation& yawPitchRoll) {
		if (slot < 0 || slot >= MAX_SLOTS) return;
		LOG_INFO("Saved slot %d", slot);

		stateSlots[slot] = {
			camera->matrix.position(),
			Quaternion::fromRotationMatrix(camera->matrix.rotation()),
			camera->fov,
			yawPitchRoll,
			true
		};
	}

	void StartLerpBetweenSlots(GameData::Camera* camera, const std::vector<uint8_t>& positionSlots) {
		slotOrder.clear();
		if (positionSlots.empty()) return;
		for (uint8_t slot : positionSlots) {
			if (slot < 0 || slot >= MAX_SLOTS) continue;
			if (stateSlots[(int)slot].isSaved) slotOrder.push_back(slot);
		}
		if (slotOrder.empty()) return;
		LOG_INFO("Interpolation started");

		isInterpolating = true;
		interval = 0;
		time = 0;
	}

	void Update(GameData::Camera* camera, Rotation& yawPitchRoll, float dt) {
		if (!isInterpolating) return;
		if (slotOrder.empty()) {
			isInterpolating = false;
			return;
		}

		if (interval + 1 >= slotOrder.size()) {
			const State& endState = stateSlots[slotOrder[interval]];
			Lerp(camera, endState, endState, 1.0f);

			yawPitchRoll = endState.yawPitchRoll;

			isInterpolating = false;
			LOG_INFO("Interpolation finished");
		}
		else {
			const State& startState = stateSlots[slotOrder[interval]];
			const State& endState = stateSlots[slotOrder[interval + 1]];

			const float t = Math::clamp(time / iTime);
			const float smootht = Math::smoothstep(t);
			Lerp(camera, startState, endState, smootht);

			if (t == 1 || startState == endState) {
				time = 0;
				++interval;
			}
			else {
				time += dt;
			}
		}
	}

private:
	void Lerp(GameData::Camera* camera, const State& startState, const State& endState, float t) {
		if (startState == endState) {
			camera->matrix.position() = startState.pos;
			camera->matrix.rotation() = startState.rotation.toRotationMatrix();
			camera->fov = startState.fov;
			return;
		}

		camera->matrix.position() = Math::lerp(startState.pos, endState.pos, t);
		camera->matrix.rotation() = Quaternion::slerp(startState.rotation, endState.rotation, t).toRotationMatrix();
		camera->fov = Math::lerp(startState.fov, endState.fov, t);
	}
};