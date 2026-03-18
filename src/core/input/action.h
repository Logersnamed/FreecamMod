#pragma once
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

#include "core/input/input.h"
#include "utils/debug.h"

class Action {
public:
	enum Type : int8_t {
		Toggle,
		ReloadConfig,
		ResetSettings,
		ExitMod,
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
		ScroolCameraSpeedModifier,
		Count
	};

private:
	Type type;

	std::vector<int> keysRequired;
	std::vector<int> keysRestricted;

public:
	Action(Type type, const std::vector<int>& keysRequired, const std::vector<int>& keysRestricted = {})
		: type(type), keysRequired(keysRequired), keysRestricted(keysRestricted) {	}

	Type GetType() const { return type; }

	bool IsPressed(const Input* input) const {
		for (int key : keysRequired) {
			if (!input->IsPressed(key)) return false;
		}
		for (int key : keysRestricted) {
			if (input->IsPressed(key)) return false;
		}
		return true;
	}

	bool IsJustPressed(const Input* input) const {
		for (int key : keysRequired) {
			if (!input->IsJustPressed(key)) return false;
		}
		for (int key : keysRestricted) {
			if (input->IsPressed(key)) return false;
		}
		return true;
	}
};

class ActionManager {
	Input* input{};
	std::unordered_map<Action::Type, Action> actionKeyMap{};

public:
	bool Initialize(Input* input) {
		if (!input) {
			Logger::Error("Input pointer is null in ActionManager::Initialize");
			return false;
		}
		this->input = input;

		return true;
	}

	bool IsPressed(Action::Type actionType) const {
		const auto action = actionKeyMap.find(actionType);
		if (action == actionKeyMap.end()) return false;
		return action->second.IsPressed(input);
	}

	bool IsJustPressed(Action::Type actionType) const {
		const auto action = actionKeyMap.find(actionType);
		if (action == actionKeyMap.end()) return false;
		return action->second.IsJustPressed(input);
	}

	void BindAction(const Action& action) {
		actionKeyMap.insert_or_assign(action.GetType(), action);
	}
};