#pragma once
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>
#include "utils/debug.h"
#include "core/input.h"

enum class ActionType : int8_t {
	Toggle,
	ReloadConfig,
	ExitMod,
	MoveForward,
	MoveBackward,
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	ZoomIn,
	ZoomOut,
	Count
};

class Action {
	ActionType type;

	std::vector<int> keysRequired;
	std::vector<int> keysRestricted;

public:
	Action(ActionType type, const std::vector<int>& keysRequired, const std::vector<int>& keysRestricted = {})
		: type(type), keysRequired(keysRequired), keysRestricted(keysRestricted) {	}

	bool IsPressed(const Input* input) const {
		if (!input) {
			Logger::Error("Input pointer is null in Action::IsPressed");
			return false;
		}
		for (int key : keysRequired) {
			if (!input->IsPressed(key)) return false;
		}
		for (int key : keysRestricted) {
			if (input->IsPressed(key)) return false;
		}
		return true;
	}

	bool IsJustPressed(const Input* input) const {
		if (!input) {
			Logger::Error("Input pointer is null in Action::IsJustPressed");
			return false;
		}
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
	std::unordered_map<ActionType, Action> actionKeyMap{};

public:
	bool Initialize(Input* input) {
		if (!input) {
			Logger::Error("Input pointer is null in ActionManager::Initialize");
			return false;
		}
		this->input = input;

		BindAction(ActionType::Toggle, { VK_F1 });
		BindAction(ActionType::ReloadConfig, { VK_RETURN });
		BindAction(ActionType::ExitMod, { VK_DELETE });
		BindAction(ActionType::MoveForward, { 'W' });
		BindAction(ActionType::MoveBackward, { 'S' });
		BindAction(ActionType::MoveLeft, { 'A' });
		BindAction(ActionType::MoveRight, { 'D' });
		BindAction(ActionType::MoveUp, { VK_SPACE });
		BindAction(ActionType::MoveDown, { VK_SHIFT });
		BindAction(ActionType::ZoomIn, { VK_OEM_PLUS });
		BindAction(ActionType::ZoomOut, { VK_OEM_MINUS });

		if (actionKeyMap.size() != static_cast<size_t>(ActionType::Count)) {
			if (actionKeyMap.size() > static_cast<size_t>(ActionType::Count)) {
				Logger::Warn("More actions defined than ActionType enum values");
			}
			else {
				Logger::Warn("Not all ActionType enum values have corresponding actions defined");
			}
		}

		return true;
	}

	bool IsPressed(ActionType actionType) const {
		const auto action = actionKeyMap.find(actionType);
		if (action == actionKeyMap.end()) return false;
		return action->second.IsPressed(input);
	}

	bool IsJustPressed(ActionType actionType) const {
		const auto action = actionKeyMap.find(actionType);
		if (action == actionKeyMap.end()) return false;
		return action->second.IsJustPressed(input);
	}

	void BindAction(ActionType actionType, const std::vector<int>& keysRequired, const std::vector<int>& keysRestricted = {}) {
		actionKeyMap.insert_or_assign(actionType, Action(actionType, keysRequired, keysRestricted));
	}
};