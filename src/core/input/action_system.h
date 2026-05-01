#pragma once
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

#include "core/input/input.h"

enum class ActionType : int8_t {
	Toggle,
	ReloadConfig,
	ResetSettings,
	ToggleFreeze,
	TeleportToCamera,
	CycleWeatherTime,
	ExitMod,
	StartEndRecording,
	StartEndPlayingRecording,
	StepFrames,
	MoveForward,
	MoveBackward,
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	Sprint,
	ZoomIn,
	ZoomOut,
	TiltLeft,
	TiltRight,
	ScrollZoomModifier,
	ScrollCameraSpeedModifier,
	ScrollSpeedhackModifier,
	ToggleSpeedhack,
	ResetSpeedhackSpeed,
	Count
};

class Action {
private:
	ActionType type;

	std::vector<int> keysRequired;
	std::vector<int> keysRestricted;

public:
	Action(ActionType type, const std::vector<int>& keysRequired, const std::vector<int>& keysRestricted = {})
		: type(type), keysRequired(keysRequired), keysRestricted(keysRestricted) {	}

	ActionType GetType() const { return type; }

	bool IsPressed(const Input& input) const {
		for (int key : keysRequired) {
			if (!input.IsPressed(key)) return false;
		}
		for (int key : keysRestricted) {
			if (input.IsPressed(key)) return false;
		}
		return true;
	}

	bool IsJustPressed(const Input& input) const {
		bool anyJustPressed = false;
		for (int key : keysRequired) {
			if (!input.IsPressed(key)) return false;
			if (input.IsJustPressed(key)) anyJustPressed = true;
		}
		for (int key : keysRestricted) {
			if (input.IsPressed(key)) return false;
		}
		return anyJustPressed;
	}
};

class ActionManager {
	std::unordered_map<ActionType, Action> actionKeyMap{};

public:
	bool IsPressed(ActionType actionType, const Input& input) const {
		const auto action = actionKeyMap.find(actionType);
		if (action == actionKeyMap.end()) return false;
		return action->second.IsPressed(input);
	}

	bool IsJustPressed(ActionType actionType, const Input& input) const {
		const auto action = actionKeyMap.find(actionType);
		if (action == actionKeyMap.end()) return false;
		return action->second.IsJustPressed(input);
	}

	void BindAction(const Action& action) {
		actionKeyMap.insert_or_assign(action.GetType(), action);
	}
};