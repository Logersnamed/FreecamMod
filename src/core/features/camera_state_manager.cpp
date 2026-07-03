#include "core/features/camera_state_manager.h"

#include "core/events.h"
#include "utils/math.h"
#include "utils/debug.h"

void CameraStateManager::SaveState(GameData::Camera* camera, int slot, const EulerAngles& yawPitchRoll) {
	if (slot < 0 || slot >= MAX_SLOTS) return;
	LOG_INFO("Saved slot %d", slot);

	stateSlots[slot] = {
		camera->matrix.position(),
		Quaternion::fromRotationMatrix(camera->matrix.rotation()),
		camera->fov,
		yawPitchRoll,
		true
	};

	EventBus::Emit(Event::SaveState{ slot, camera->matrix.position(), });
}

void CameraStateManager::StartLerpBetweenSlots(GameData::Camera* camera, Input::ReleasedNumkeys& positionSlots) {
	slotOrder.clear();
	if (positionSlots.empty()) return;
	for (uint8_t slot : positionSlots) {
		if (slot >= MAX_SLOTS) continue;
		if (stateSlots[(int)slot].isSaved) slotOrder.push_back(slot);
	}
	if (slotOrder.empty()) return;
	LOG_INFO("Interpolation started");

	isInterpolating = true;
	interval = 0;
	time = 0;

	EventBus::Emit(Event::Interpolate{ .isEnabled = true, .slots = slotOrder });
}

void CameraStateManager::Update(GameData::Camera* camera, EulerAngles* yawPitchRoll, float dt) {
	if (!isInterpolating) return;
	if (slotOrder.empty()) {
		isInterpolating = false;
		return;
	}

	if (interval + 1 >= slotOrder.size()) {
		const State& endState = stateSlots[slotOrder[interval]];
		Lerp(camera, endState, endState, 1.0f);

		if (yawPitchRoll) *yawPitchRoll = endState.yawPitchRoll;

		isInterpolating = false;
		LOG_INFO("Interpolation finished");
		EventBus::Emit(Event::Interpolate{ .isEnabled = false });
	}
	else {
		const State& startState = stateSlots[slotOrder[interval]];
		const State& endState = stateSlots[slotOrder[interval + 1]];

		const float t = Math::clamp(time / interpolationTime);
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

void CameraStateManager::Lerp(GameData::Camera* camera, const State& startState, const State& endState, float t) {
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