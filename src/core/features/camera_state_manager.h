#pragma once
#include <iostream>
#include <vector>

#include "core/game_data/game_data.h"
#include "utils/types.h"
#include "utils/math.h"
#include "utils/debug.h"

class CameraStateManager {
	struct State {
		float3 pos = 0.0f;
		matrix3x3 rotation{};
		float fov = 1.0f;

		bool operator==(const State& other) const {
			return pos == other.pos && rotation == other.rotation && fov == other.fov;
		}
	} stateSlots[10]{};

	bool isInterpolating = false;
	std::vector<uint8_t> slotOrder;
	uint8_t interval = 0;
	float time = 0;
	const int iTime = 3.0f;

public:
	void SaveState(GameData::Camera* camera, int slot) {
		Logger::Info("Saved sloat %d", slot);

		State state = {
			camera->matrix.position(),
			(matrix3x3)camera->matrix.rotation(),	// is not saved correctly because it doenst account my pitchyaws or smth
			camera->fov
		};

		stateSlots[slot] = state;
	}

	void StartLerpBetweenSlots(GameData::Camera* camera, const std::vector<uint8_t>& positionSlots) {
		Logger::Info("Interpolation started");
		if (positionSlots.size() == 1) {
			State& state = stateSlots[positionSlots[0]];
			camera->matrix.position() = state.pos;
			camera->matrix.rotation() = state.rotation;
			camera->fov = state.fov;
			return;
		}

		slotOrder = positionSlots;

		for (auto& slot : slotOrder)
			Logger::Info("%d", slot);

		isInterpolating = true;
		interval = 0;
		time = 0;
	}

	void Update(GameData::Camera* camera,  float dt) {
		if (!isInterpolating) return;

		if (interval + 1 >= slotOrder.size()) {
			const State& endState = stateSlots[slotOrder[interval]];
			Lerp(camera, endState, endState, 1.0f);

			isInterpolating = false;
			Logger::Info("Interpolation finished");
		}
		else {
			const State& startState = stateSlots[slotOrder[interval]];
			const State& endState = stateSlots[slotOrder[interval + 1]];

			const float t = Math::clamp(time / iTime);
			const float smootht = Math::smoothstep(t);
			Lerp(camera, startState, endState, smootht);

			if (t == 1) {
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
			camera->matrix.rotation() = startState.rotation;
			camera->fov = startState.fov;
			return;
		}

		camera->matrix.position() = Math::lerp(startState.pos, endState.pos, t);
		camera->matrix.rotation() = endState.rotation; /*Math::lerp(startState.rotation, endState.rotation, t);*/
		camera->fov = Math::lerp(startState.fov, endState.fov, t);
	}
};