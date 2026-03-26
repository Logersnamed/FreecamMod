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
		Quaternion rotation{};
		float fov = 1.0f;
		float3 yawPitchRoll = 0;

		bool operator==(const State& other) const { return pos == other.pos && rotation == other.rotation && fov == other.fov; }
	} stateSlots[10]{};

	bool isInterpolating = false;
	std::vector<uint8_t> slotOrder;	
	uint8_t interval = 0;
	float time = 0;
	const int iTime = 3.0f;

public:
	void SaveState(GameData::Camera* camera, int slot, float3 yawPitchRoll) {
		Logger::Info("Saved slot %d", slot);

		State state = {
			camera->matrix.position(),
			Quaternion::fromRotationMatrix(camera->matrix.rotation()),
			camera->fov,
			yawPitchRoll
		};

		stateSlots[slot] = state;
	}

	void StartLerpBetweenSlots(GameData::Camera* camera, const std::vector<uint8_t>& positionSlots) {
		Logger::Info("Interpolation started");
		slotOrder = positionSlots;

		isInterpolating = true;
		interval = 0;
		time = 0;
	}

	void Update(GameData::Camera* camera, const float3_ref& yawPitchRoll, float dt) {
		if (!isInterpolating) return;

		if (interval + 1 >= slotOrder.size()) {
			const State& endState = stateSlots[slotOrder[interval]];
			Lerp(camera, endState, endState, 1.0f);

			yawPitchRoll.x = endState.yawPitchRoll.x;
			yawPitchRoll.y = endState.yawPitchRoll.y;
			yawPitchRoll.z = endState.yawPitchRoll.z;

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
			camera->matrix.rotation() = startState.rotation.toRotationMatrix();
			camera->fov = startState.fov;
			return;
		}

		camera->matrix.position() = Math::lerp(startState.pos, endState.pos, t);
		camera->matrix.rotation() = Quaternion::slerp(startState.rotation, endState.rotation, t).toRotationMatrix();
		camera->fov = Math::lerp(startState.fov, endState.fov, t);
	}
};