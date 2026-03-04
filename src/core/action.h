#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include "utils/debug.h"
#include "core/input.h"

enum class ActionType {
	Toggle,
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

private:
	ActionType type;
	std::string name; // TODO

	std::vector<int> keysRequired;
	std::vector<int> keysRestricted;
};

class ActionManager {
public:
	bool Initialize(Input* input) {
		if (!input) {
			Logger::Error("Input pointer is null in ActionManager::Initialize");
			return false;
		}
		this->input = input;

		AddAction(ActionType::Toggle, { VK_F1 });
		AddAction(ActionType::MoveForward, { 'W' });
		AddAction(ActionType::MoveBackward, { 'S' });
		AddAction(ActionType::MoveLeft, { 'A' });
		AddAction(ActionType::MoveRight, { 'D' });
		AddAction(ActionType::MoveUp, { VK_SPACE });
		AddAction(ActionType::MoveDown, { VK_SHIFT });
		AddAction(ActionType::ZoomIn, { VK_OEM_PLUS });
		AddAction(ActionType::ZoomOut, { VK_OEM_MINUS });

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

private:
	Input* input{};
	std::unordered_map<ActionType, Action> actionKeyMap{};

	void AddAction(ActionType actionType, const std::vector<int>& keysRequired, const std::vector<int>& keysRestricted = {}) {
		actionKeyMap.insert({ actionType, Action(actionType, keysRequired, keysRestricted) });
	}
};