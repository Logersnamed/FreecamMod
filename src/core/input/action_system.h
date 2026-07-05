#pragma once
#include <cstdint>
#include <array>
#include <optional>
#include <string>

#include "core/events.h"
#include "core/input/action_type.h"
#include "core/input/input.h"
#include "utils/types.h"

class Action {
public:
	static constexpr int MAX_KEYS = 4;
	using Keys = FixedVec<int, MAX_KEYS>;

private:
	ActionType type;
	Keys keysRequired;
	Keys keysRestricted;

public:
	Action(ActionType type, const Keys& keysRequired, const Keys& keysRestricted = {})
		: type(type), keysRequired(keysRequired), keysRestricted(keysRestricted) {	}

	ActionType GetType() const { return type; }
	const Keys& GetRequiredKeys() const { return keysRequired; }
	const Keys& GetRestrictedKeys() const { return keysRestricted; }
	const char* GetName() const { return ActionTypeName[static_cast<int8_t>(type)]; }

	bool IsPressed(const Input& input) const {
		if (keysRequired.empty() && keysRestricted.empty()) return false;
		for (int key : keysRequired) {
			if (!input.IsPressed(key)) return false;
		}
		for (int key : keysRestricted) {
			if (input.IsPressed(key)) return false;
		}
		return true;
	}

	bool IsJustPressed(const Input& input) const {
		if (keysRequired.empty() && keysRestricted.empty()) return false;
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

private:
	static inline const char* ActionTypeName[] = {
		#define X(name) #name,
		ACTION_TYPES
		#undef X
		"Count"
	};
};

class ActionManager {
	std::array<std::optional<Action>, static_cast<size_t>(ActionType::Count)> actions{};

	const Input& input;

public:	
	ActionManager(const Input& input) : input(input) {}

	bool IsPressed(ActionType actionType) const {
		const auto& action = actions[static_cast<size_t>(actionType)];
		return action.has_value() && action->IsPressed(input);
	}

	bool IsJustPressed(ActionType actionType) const {
		const auto& action = actions[static_cast<size_t>(actionType)];
		return action.has_value() && action->IsJustPressed(input);
	}

	void BindAction(const Action& action) {
		actions[static_cast<size_t>(action.GetType())] = action;
	}
};