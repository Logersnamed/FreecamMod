#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cassert>
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
};

class Action {
public:
	Action(ActionType type, const std::vector<int>& keysRequired, const std::vector<int>& keysRestricted = {})
		: type(type), keysRequired(keysRequired), keysRestricted(keysRestricted) {	}

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
		for (int key : keysRequired) {
			if (!input.IsJustPressed(key)) return false;
		}
		for (int key : keysRestricted) {
			if (input.IsPressed(key)) return false;
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
	ActionManager() {
		AddAction(ActionType::Toggle, { VK_F1 });
		AddAction(ActionType::MoveForward, { 'W' });
		AddAction(ActionType::MoveBackward, { 'S' });
		AddAction(ActionType::MoveLeft, { 'A' });
		AddAction(ActionType::MoveRight, { 'D' });
		AddAction(ActionType::MoveUp, { VK_SPACE });
		AddAction(ActionType::MoveDown, { VK_SHIFT });
		AddAction(ActionType::ZoomIn, { VK_OEM_PLUS });
		AddAction(ActionType::ZoomOut, { VK_OEM_MINUS });

		assert(sizeof(ActionType) / sizeof(ActionType::Toggle) == sizeof(actionKeyMap));
	}

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

private:
	std::unordered_map<ActionType, Action> actionKeyMap{};

	void AddAction(ActionType actionType, const std::vector<int>& keysRequired, const std::vector<int>& keysRestricted = {}) {
		actionKeyMap.insert({ actionType, Action(actionType, keysRequired, keysRestricted) });
	}
};